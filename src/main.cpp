
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

static const int boob = 3;
template<int I>
class MyTemplate {};

static const MyTemplate<(boob<1>3)> poop;

std::vector<token> pp_tokens;
std::vector<token> tokens;

inline bool tok_name_match(const token& tok, const char* name) {
    size_t len = strlen(name);
    if(tok.length != len) {
        return false;
    }
    return strncmp(tok.string, name, len) == 0;
}

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

// Tokenize buffer for preprocessing (include whitespace and newline)
bool tokenize(const std::vector<char>& buffer, std::vector<token>& tokens) {

}

int main(int argc, char** argv) {
    if(argc < 2) {
        return 1;
    }
    std::string fname = argv[1];

    printf("Parsing file %s...\n", fname.c_str());
    
    std::vector<char> buffer;
    if(!load_file(fname.c_str(), buffer)) {
        return 1;
    }

    // Erase all preprocessor directives
    preprocess(buffer.data(), buffer.size());
    std::ofstream f(fname + ".prep", std::ios::binary);
    f.write(buffer.data(), buffer.size());
    f.close();

    // Tokenize
    {
        enum tokenizer_state {
            tstate_default,
            tstate_identifier,
            tstate_integer,
            tstate_numreal,
            tstate_literal,
            tstate_dot,
            tstate_string_constant,
            tstate_char_constant,
            tstate_fwd_slash,
            tstate_comment_line,
            tstate_comment_multiline,
            tstate_comment_multiline_end
        };
        tokenizer_state tstate = tstate_default;
        token tok;
        size_t cid_start = 0;
        size_t cid = 0;
        size_t line = 1, column = 0;
        size_t column_start = 0; // starting column for current token
        char c = buffer[cid];
        char cc = buffer[cid + 1];
        char ccc = buffer[cid + 2];
        auto advance = [&c, &cc, &ccc, &buffer, &cid, &column](){
            c = buffer[++cid];
            cc = buffer[cid + 1];
            ccc = buffer[cid + 2];
            ++column;
        };
        auto submit_token = [&tok, &buffer, &cid, &cid_start, &column_start, &column, &line](token_type type){
            tok.string = &buffer[cid_start];
            tok.length = cid - cid_start;
            tok.type = type;
            tok.line = line;
            tok.col = column_start;
            if(type != tok_comment) tokens.push_back(tok);
            cid_start = cid;

            column_start = column;
        };
        while(c != '\0') {
            switch(tstate) {
            case tstate_default:
                tok.string = &buffer[cid];
                tok.length = 0;
                cid_start = cid;
                column_start = column;
                if(c == '\\' && isnewline(cc)) {
                    line++;
                    column = 0;
                    advance();
                    advance();
                } else if(isnewline(c)) {
                    line++;
                    column = 0;
                    advance();
                } else if(isalpha(c)) { tstate = tstate_identifier; }
                else if(isnum(c)) { tstate = tstate_integer; }
                else if(c == ';') { advance(); submit_token(tok_semicolon); }
                else if(c == '{') { advance(); submit_token(tok_brace_l); }
                else if(c == '}') { advance(); submit_token(tok_brace_r); }
                else if(c == '[') { 
                    advance();
                    submit_token(tok_bracket_l);
                } else if(c == ']') { 
                    advance();
                    submit_token(tok_bracket_r);
                } else if(c == '(') { advance(); submit_token(tok_paren_l); }
                else if(c == ')') { advance(); submit_token(tok_paren_r); }
                else if(c == '#') { advance(); submit_token(tok_hash); }
                else if(c == '\"') { tstate = tstate_string_constant; }
                else if(c == '\'') { tstate = tstate_char_constant; }
                else if(c == '=') {
                    advance();
                    if(c == '=') {  // ==
                        advance();
                        submit_token(tok_equals);
                    } else {        // =
                        submit_token(tok_assign);
                    }
                } else if(c == '+') { // +, ++, +=
                    advance();
                    if(c == '+') {
                        advance();
                        submit_token(tok_incr);
                    } else if(c == '=') {
                        advance();
                        submit_token(tok_plus_assign);
                    } else {
                        submit_token(tok_plus);
                    }
                } else if(c == '-') { // -, --, -=, ->, ->*
                    advance();
                    if(c == '-') {
                        advance();
                        submit_token(tok_decr);
                    } else if(c == '=') {
                        advance();
                        submit_token(tok_minus_assign);
                    } else if(c == '>') {
                        advance();
                        if(c == '*') {
                            advance();
                            submit_token(tok_arrow_member);
                        } else {
                            submit_token(tok_arrow);
                        }
                    }  
                } else if(c == '*') { // *, *=
                    advance();
                    if(c == '=') {
                        advance();
                        submit_token(tok_asterisk_assign);
                    } else {
                        submit_token(tok_asterisk);
                    }
                } else if(c == '/') {  // /, /=, //, /*
                    tstate = tstate_fwd_slash; 
                } else if(c == '%') { // %, %=
                    advance();
                    if(c == '=') {
                        advance();
                        submit_token(tok_percent_assign);
                    } else {
                        submit_token(tok_percent);
                    }
                } else if(c == '!') { // !, !=
                    advance();
                    if(c == '=') {
                        advance();
                        submit_token(tok_excl_assign);
                    } else {
                        submit_token(tok_excl);
                    }
                } else if(c == '<') { // <, <=, <=>, <<, <<=
                    advance();
                    if(c == '=') {
                        advance();
                        if(c == '>') {
                            advance();
                            submit_token(tok_three_way_comp);
                        } else {
                            submit_token(tok_less_assign);
                        }
                    } else if(c == '<') {
                        advance();
                        if(c == '=') {
                            advance();
                            submit_token(tok_shift_left_assign);
                        } else {
                            submit_token(tok_shift_left);
                        }
                    } else {
                        submit_token(tok_less);
                    }
                } else if(c == '>') { // >, >=, >>, >>=
                    advance();
                    if(c == '=') {
                        advance();
                        submit_token(tok_more_assign);
                    } else if(c == '>') {
                        advance();
                        if(c == '=') {
                            advance();
                            submit_token(tok_shift_right_assign);
                        } else {
                            submit_token(tok_shift_right);
                        }
                    } else {
                        submit_token(tok_more);
                    }
                } else if(c == '&') { // &, &&, &=
                    advance();
                    if(c == '&') {
                        advance();
                        submit_token(tok_double_amp);
                    } else if(c == '=') {
                        advance();
                        submit_token(tok_amp_assign);
                    } else {
                        submit_token(tok_amp);
                    }
                } else if(c == '|') { // |, ||, |=
                    advance();
                    if(c == '|') {
                        advance();
                        submit_token(tok_double_pipe);
                    } else if(c == '='){
                        advance();
                        submit_token(tok_pipe_assign);
                    } else {
                        submit_token(tok_pipe);
                    }
                } else if(c == '~') { // ~
                    advance();
                    submit_token(tok_tilde);
                } else if(c == '^') { // ^, ^=
                    advance();
                    if(c == '=') {
                        advance();
                        submit_token(tok_hat_assign);
                    } else {
                        submit_token(tok_hat);
                    }
                } else if(c == ',') { // ,
                    advance();
                    submit_token(tok_comma);
                } else if(c == '?') { // ?
                    advance();
                    submit_token(tok_question);
                } else if(c == ':') { // :, ::
                    advance();
                    if(c == ':') {
                        advance();
                        submit_token(tok_double_colon);
                    } else {
                        submit_token(tok_colon);
                    }
                } else if(c == '.') { tstate = tstate_dot; }
                else if(c == '/') { tstate = tstate_fwd_slash; }
                else advance();
                break;
            case tstate_fwd_slash:
                advance();
                if(c == '/') {
                    tstate = tstate_comment_line;
                } else if(c == '*') {
                    tstate = tstate_comment_multiline;
                } else if(c == '=') {
                    advance();
                    submit_token(tok_fwd_slash_assign);
                    tstate = tstate_default;
                } else {
                    submit_token(tok_fwd_slash);
                    tstate = tstate_default;
                }
                break;
            case tstate_comment_line:
                advance();
                if(c == '\n') {
                    submit_token(tok_comment);
                    tstate = tstate_default;
                }
                break;
            case tstate_comment_multiline:
                advance();
                if(c == '*') {
                    advance();
                    if(c == '/') {
                        advance();
                        submit_token(tok_comment);
                        tstate = tstate_default;
                    }
                }
                break;
            case tstate_identifier:
                advance();
                if(!isalphanum(c)) {
                    submit_token(tok_identifier);
                    tstate = tstate_default;
                }
                break;
            case tstate_integer:
                advance();
                if(isnum(c)) {
                    continue;
                } else if(isalpha(c)) {
                    submit_token(tok_int_literal);
                    tstate = tstate_literal;
                } else if(c == '.') {
                    tstate = tstate_numreal;
                } else {
                    submit_token(tok_int_literal);
                    tstate = tstate_default;
                }
                break;
            case tstate_numreal:
                advance();
                if(isnum(c)) {
                    continue;
                } else if(isalpha(c)) {
                    submit_token(tok_float_literal);
                    tstate = tstate_literal;
                } else {
                    submit_token(tok_float_literal);
                    tstate = tstate_default;
                }
                break;
            case tstate_dot:
                advance();
                if(isnum(c)) {
                    tstate = tstate_numreal;
                } else if(c == '*') {
                    advance();
                    submit_token(tok_dot_asterisk);
                    tstate = tstate_default;
                } else if(c == '.') {
                    advance();
                    if(c == '.') {
                        advance();
                        submit_token(tok_elipsis);
                        tstate = tstate_default;
                    } else {
                        assert(false);
                    }
                } else {
                    submit_token(tok_dot);
                    tstate = tstate_default;
                }
                break;
            case tstate_literal:
                advance();
                if(isalpha(c)) {
                    continue;
                } else {
                    submit_token(tok_literal);
                    tstate = tstate_default;
                }
                break;
            case tstate_string_constant:
                advance();
                if(c != '\"' && !isnewline(c) && !iseof(c)) {
                    continue;
                } else {
                    advance();
                    submit_token(tok_string_constant);
                    tstate = tstate_default;
                }
                break;
            case tstate_char_constant:
                advance();
                if(c != '\'' && !isnewline(c) && !iseof(c)) {
                    continue;
                } else {
                    advance();
                    submit_token(tok_char_constant);
                    tstate = tstate_default;
                }
                break;
            };
        }
    }
    
    token tok;
    tok.type = tok_eof;
    tokens.push_back(tok);

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
