
#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <fstream>
#include <functional>

#include "preprocess.hpp"
#include "token.hpp"
#include "node.hpp"
#include "file_data.hpp"

#include "tokenize.hpp"

#define LOL <stdint.h>
#define TEST(B) static const int A = int(B);
#define TEST2 inline int foobar
#define TEST3(A) static const char* a## b = #  A;
#define TEST4(A, B) static const int A ## B;

#define MY_MACRO static_assert(false);
#define TEST5(A, B)A ## _ ## B

TEST2 (int value){};
TEST( );
TEST3(hello)
TEST4(, Bar)

static const int boob = 3;
template<int I>
class MyTemplate {};

static const MyTemplate<(boob<1>3)> poop;

std::vector<token> pp_tokens;
std::vector<token> tokens;

void error(const char* text, const token& tok) {
    printf("error: %s, line %i (%s)\n", text, tok.line, tok.get_string().c_str());
}
void error(const char* text) {
    printf("error: %s\n", text);
}

// Load entire text file into a char buffer
bool load_file(const char* fname, std::vector<char>& buffer) {
    std::ifstream file(fname);
    if (!file.is_open()) {
        printf("Failed to open file\n");
        return false;
    }
    file.seekg(0, std::ios::end); // go to the end
    size_t length = file.tellg(); // report location (this is the length)
    file.seekg(0, std::ios::beg);
    
    buffer.resize(length);
    file.read((char*)buffer.data(), length);
    buffer.push_back('\0');
    buffer.push_back('\0');
    return true;
}

bool load_file2(const char* fname, std::vector<char>& buffer) {
    buffer.clear();
    std::ifstream file(fname);
    if(!file.is_open()) {
        printf("Failed to open file %s\n", fname);
        return false;
    }
    for(std::istreambuf_iterator<char> it(file), end; it != end; ++it) {
        char c = *it;        
        switch(c) {
        case '\\': {
            ++it;
            if(it == end) {
                buffer.push_back('\n');
                break;
            }
            char next = *it;
            if(next == '\n') {
                break;
            }
            buffer.push_back('\\');
        }
        default:
            buffer.push_back(c);
        }
    }
    return true;
}

void dump_buffer(const std::vector<char>& buffer, const char* fname) {
    std::ofstream f(fname, std::ios::binary);
    f.write(buffer.data(), buffer.size());
    f.close();
}
void dump_tokens(const std::vector<token>& tokens, const char* fname) {
    std::ofstream f(fname, std::ios::binary);
    for(auto& t : tokens) {
        f.write(t.string, t.length);
    }
    f.close();
}

int main(int argc, char** argv) {
    if(argc < 2) {
        return 1;
    }
    std::string fname = argv[1];

    printf("Parsing file %s...\n", fname.c_str());
    
    std::vector<char> buffer;
    if(!load_file2(fname.c_str(), buffer)) {
        return 1;
    }
    dump_buffer(buffer, (fname + ".phase2").c_str());

    //std::vector<char> buffer_preprocessed;
    //remove_pp(buffer.data(), buffer.size(), buffer_preprocessed);
    //dump_buffer(buffer_preprocessed, (fname + ".pp").c_str());

    tokenize(buffer, pp_tokens);
    std::vector<char> preprocessed_buffer;
    preprocess(pp_tokens, preprocessed_buffer);
    dump_buffer(preprocessed_buffer, (fname + ".pp").c_str());
    return 0;

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

    {
        token* tok = &tokens.front();
        size_t tid = 0;
        size_t start_tid = 0;
        std::string parse_error_text;
        auto advance = [&tok, &tid](){
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
            print_seq(root.get());
        }

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
    
    return 0;
}
