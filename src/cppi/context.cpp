#include "context.hpp"

#include <functional>

#include "tokenize.hpp"
#include "parse_node.hpp"
#include "file_util.hpp"

#include <windows.h>


namespace cppi {

bool context::parse(const char* fname) {
    std::vector<char> buf;
    if(!load_file(fname, buf)) {
        return false;
    }

    char fname_buf[MAX_PATH];
    memset(fname_buf, 0, MAX_PATH);
    char* fname_part = 0;
    auto len = GetFullPathNameA(fname, MAX_PATH, fname_buf, &fname_part);
    if(len == 0) {
        assert(false);
        return 1;
    }
    std::string full_fpath(fname_buf, fname_buf + len);
    std::string full_path_no_name(fname_buf, fname_part);

    return parse(buf.data(), buf.size(), full_fpath.c_str());
}

bool context::parse(const char* buffer, size_t length, const char* full_file_name_hint) {
    if(!pp_ctx.preprocess(buffer, length, full_file_name_hint)) {
        return false;
    }

    // Tokenize
    if(pp_ctx.get_preprocessed_length() == 0) {
        assert(false);
        return false;
    }
    std::vector<char> preprocessed_buffer(
        pp_ctx.get_preprocessed_buffer(), 
        pp_ctx.get_preprocessed_buffer() + pp_ctx.get_preprocessed_length()
    );
    std::vector<token> tokens;
    if(!tokenize(preprocessed_buffer, tokens, true)) {
        return false;
    }

    // Refine tokens using known keywords
    std::map<const char*, token_type> known_keyword_to_token = {
        { "enum", tok_enum },
        { "struct", tok_struct },
        { "class", tok_class },
        { "namespace", tok_namespace },

        // Storage class specifiers
        { "register", tok_register },
        { "static", tok_static },
        { "thread_local", tok_thread_local },
        { "extern", tok_extern },
        { "mutable", tok_mutable },

        // Function specifiers
        { "inline", tok_inline },
        { "virtual", tok_virtual },
        { "explicit", tok_explicit },

        // Other
        { "typedef", tok_typedef },
        { "friend", tok_friend },
        { "constexpr", tok_constexpr },

        // Type specifiers
        // cv-qualifiers
        { "const", tok_const },
        { "volatile", tok_volatile },

        // Simple type specifiers
        { "char", tok_char },
        { "char16_t", tok_char16_t },
        { "char32_t", tok_char32_t },
        { "wchar_t", tok_wchar_t },
        { "bool", tok_bool },
        { "short", tok_short },
        { "int", tok_int },
        { "long", tok_long },
        { "signed", tok_signed },
        { "unsigned", tok_unsigned },
        { "float", tok_float },
        { "double", tok_double },
        { "void", tok_void },
        { "auto", tok_auto },
        { "decltype", tok_decltype },

        //
        { "static_assert", tok_static_assert },
        { "using", tok_using },

        //
        { "asm", tok_asm },

        { "private", tok_private },
        { "protected", tok_protected },
        { "public", tok_public },

        { "final", tok_final },
        { "override", tok_override },

        { "noexcept", tok_noexcept },
        { "throw", tok_throw }
    };
    for(int i = 0; i < tokens.size(); ++i) {
        auto& tok = tokens[i];
        if(tok.type == tok_identifier) {
            // TODO: add token 'subtype'
            for(auto& kv : known_keyword_to_token) {
                if(tok_name_match(tok, kv.first)) {
                    tok.type = kv.second;
                }
            }
        }
    }

    // Build a node tree
    // all nested {}, [] and () are converted to single nodes
    // this allows for easy skipping of things like function bodies
    {
        token* tok = &tokens.front();
        size_t tid = 0;
        size_t start_tid = 0;
        std::string parse_error_text;
        auto advance = [&tokens, &tok, &tid](){
            tok = &tokens[++tid];
        };

        // -----------------------------------------------------
        
        std::unique_ptr<node> root;
        root.reset(new node(node_token_seq, 0));
        node* current = root.get();
        current->token_first = 0;
        current->token_count = tokens.size();

        {
            std::vector<int> paren_stack;
            
            while(tok->type != tok_eof) {
                if(tok->type == tok_brace_l) {
                    current->nodes.push_back(std::unique_ptr<node>(new node(node_brace_block, current)));
                    current = current->nodes.back().get();;
                    paren_stack.push_back(tid);
                    // Brace open
                } else if(tok->type == tok_bracket_l) {
                    current->nodes.push_back(std::unique_ptr<node>(new node(node_bracket_block, current)));
                    current = current->nodes.back().get();
                    paren_stack.push_back(tid);
                    // Bracket open
                } else if(tok->type == tok_paren_l) {
                    current->nodes.push_back(std::unique_ptr<node>(new node(node_paren_block, current)));
                    current = current->nodes.back().get();
                    paren_stack.push_back(tid);
                    // Parenthesis open
                } else if(tok->type == tok_brace_r) {
                    if(paren_stack.empty() || tokens[paren_stack.back()].string[0] != '{') {
                        printf("error - unexpected }\n");
                        break;
                    } else {
                        current->token_first = paren_stack.back();
                        current->token_count = tid - paren_stack.back() + 1;
                        current = current->parent;
                        paren_stack.pop_back();
                        // Brace closed
                    }
                } else if(tok->type == tok_bracket_r) {
                    if(paren_stack.empty() || tokens[paren_stack.back()].string[0] != '[') {
                        printf("error - unexpected ]\n");
                        break;
                    } else {
                        current->token_first = paren_stack.back();
                        current->token_count = tid - paren_stack.back() + 1;
                        current = current->parent;
                        paren_stack.pop_back();
                        // Bracket closed
                    }
                } else if(tok->type == tok_paren_r) {
                    if(paren_stack.empty() || tokens[paren_stack.back()].string[0] != '(') {
                        printf("error - unexpected )\n");
                        break;
                    } else {
                        current->token_first = paren_stack.back();
                        current->token_count = tid - paren_stack.back() + 1;
                        current = current->parent;
                        paren_stack.pop_back();
                        // Parenthesis closed
                    }
                } else {
                    current->nodes.push_back(std::unique_ptr<node>(new node(node_token, current)));
                    current->nodes.back()->token_first = tid;
                    current->nodes.back()->token_count = 1;
                    current->nodes.back()->tok = &tokens[tid];
                    // Single token
                }
                advance();
            }
            if(!paren_stack.empty()) {
                printf("error - %c not closed\n", paren_stack.back());
            }

            std::function<void(node*)> print_seq;
            print_seq = [&print_seq](node* n) {
                for(auto& ch : n->nodes) {
                    if(ch->type == node_token) {
                        printf("%s\n", std::string(ch->tok->string, ch->tok->length).c_str());
                    } else if(ch->type == node_brace_block) {
                        printf("{ ... }\n");
                    } else if(ch->type == node_bracket_block) {
                        printf("[ ... ]\n");
                    } else if(ch->type == node_paren_block) {
                        printf("( ... )\n");
                    }
                }
            };
            //print_seq(root.get());
        }

        // Parse
        {
            node_cursor cursor(root.get());
            while(cursor) {
                int adv = try_class_specifier(cursor);
                if(!adv) adv = try_simple_declaration(cursor);
                if(!adv) adv = try_function_definition(cursor);
                if(adv) {
                    cursor.advance(adv);
                } else {
                    while(!cursor.is_node(node_brace_block) 
                    && !cursor.is_token(tok_semicolon)
                    && !cursor.is_token(tok_eof)) {
                        cursor.advance();
                    }
                    if(cursor.is_token(tok_eof)) {
                        break;
                    }
                    cursor.advance();
                }
            }
        }
    }

    return true;
}

}