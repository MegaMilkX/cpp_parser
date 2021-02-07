#ifndef PREPROCESS_HPP
#define PREPROCESS_HPP

#include <vector>
#include <stdio.h>
#include "tokenize.hpp"
#include "util.hpp"

struct pp_line {
    char* buf;
    size_t len;
};

inline void print_tok(const token& tok) {
    printf("%s", tok.get_string().c_str());
}

static const auto pp_error = [](const char* format, ...){
    va_list _ArgList;
    __crt_va_start(_ArgList, format);
    printf((std::string("PP ERROR: ") + format).c_str(), _ArgList);
    __crt_va_end(_ArgList);
};

struct pp_macro {
    std::string name;
    bool has_parameter_list = false;
    bool has_variadic_param = false;
    std::vector<token> parameters;
    std::vector<token> replacement_list;
};
std::map<std::string, pp_macro> macros;
std::vector<std::string> expansion_stack; // to track circular expansion

inline std::vector<char> pp_string_from_tokens(const std::vector<token>& tokens) {
    std::vector<char> out;

    for(int i = 0; i < tokens.size(); ++i) {
        auto t = tokens[i];
        if(t.type == tok_whitespace || t.type == tok_newline) {
            while(t.type == tok_whitespace || t.type == tok_newline) {
                ++i;
                if(i == tokens.size() - 1) {
                    break;
                }
                t = tokens[i];
            }
            out.push_back(' ');
            --i;
        } else {
            out.insert(out.end(), t.string, t.string + t.length);
        }
    }
    if(!out.empty() && out.back() == ' ') {
        out.pop_back(); 
    }

    return out;
}

inline std::vector<char> pp_string_constant_from_tokens(const std::vector<token>& tokens) {
    // TODO: Unfinished, check standard
    std::vector<char> out;
    
    token tok = tokens[0];
    size_t cur = 0;
    auto advance = [&tokens, &tok, &cur](){
        if(cur == tokens.size() - 1) { tok.type = tok_eof;}
        else { tok = tokens[++cur]; }
    };
    auto is_tok = [&tok](token_type type) -> bool { return tok.type == type; };
    auto emit_and_advance = [&advance, &tok, &out](){
        out.insert(out.end(), tok.string, tok.string + tok.length);
        advance();
    };

    out.push_back('"');
    while(!is_tok(tok_eof)) {
        if(is_tok(tok_whitespace) || is_tok(tok_newline)) {
            while(is_tok(tok_whitespace) || is_tok(tok_newline)) {
                advance();
            }
            out.push_back(' ');
            continue;
        }
        if(is_tok(tok_string_constant)) {
            out.push_back(92); // \ 
            out.push_back(34); // "
            out.insert(out.end(), tok.string + 1, tok.string + tok.length - 1);
            out.push_back(92); // \ 
            out.push_back(34); // "
            advance();
            continue;
        }
        emit_and_advance();
    }
    out.push_back('"');

    return out;
}

inline bool preprocess(const std::vector<token>& tokens, std::vector<char>& out_buf);

inline bool process_replacement_list(const pp_macro& macro, std::vector<char>& out, const std::vector<std::vector<token>>& macro_args = std::vector<std::vector<token>>(), const std::vector<token>& va_args = std::vector<token>()) {
    if(macro.replacement_list.empty()) {
        return true;
    }
    
    token tok = macro.replacement_list[0];
    size_t cur = 0;
    auto advance = [&macro, &tok, &cur](){
        if(cur == macro.replacement_list.size() - 1) { tok.type = tok_eof;}
        else { tok = macro.replacement_list[++cur]; }
    };
    auto reset = [&macro, &tok, &cur](){
        tok = macro.replacement_list[0];
        cur = 0;
    };
    auto is_tok = [&tok](token_type type) -> bool { return tok.type == type; };
    auto get_param_idx = [&is_tok, &macro, &tok]() -> int {
        if(!is_tok(tok_identifier)) {
            return -1;
        }
        if(!macro.has_parameter_list) {
            return -1;
        }
        int idx = 0;
        for(auto& param : macro.parameters) {
            if(param.get_string().compare(tok.get_string()) == 0) {
                return idx;
            }
            ++idx;
        }
        return -1;
    };
    auto emit_tok = [&out](const token& tok){
        out.insert(out.end(), tok.string, tok.string + tok.length);
    };
    auto emit_and_advance = [&advance, &tok, &out](){
        out.insert(out.end(), tok.string, tok.string + tok.length);
        advance();
    };
    auto emit_buf = [&out](const std::vector<char>& str){
        out.insert(out.end(), str.begin(), str.end());
    };

    // handle # and ## operators, and expand parameters
    int param_idx = 0;
    while(!is_tok(tok_eof)) {
        if(is_tok(tok_hash)) {
            advance();
            int param_idx = get_param_idx();
            if(param_idx < 0) {
                pp_error("token after # must be a macro parameter");
                return false;
            }
            std::vector<char> stringified_arg;
            if(param_idx >= macro_args.size()) {
                // TODO: WARNING: Not enough parameters
                stringified_arg = { '"', '"' };
            } else {
                stringified_arg = pp_string_constant_from_tokens(macro_args[param_idx]);                
            }
            emit_buf(stringified_arg);
            advance();
        } else if(tok.get_string() == "__VA_ARGS__") {
            std::vector<char> str = pp_string_from_tokens(va_args);
            emit_buf(str);
            advance();
        } else if((param_idx = get_param_idx()) >= 0) {
            if(param_idx >= macro_args.size()) {
                // TODO: WARNING: Not enough parameters
                // do nothing
                advance();
            } else {
                std::vector<char> str = pp_string_from_tokens(macro_args[param_idx]);
                emit_buf(str);
                advance();
            }
        } else {
            emit_and_advance();
        }

        if(is_tok(tok_double_hash)) {
            advance();
        } else if(is_tok(tok_whitespace)) {
            token whitespace_tok = tok;
            advance();
            if(is_tok(tok_double_hash)) {
                advance();
                if(is_tok(tok_whitespace)) {
                    advance();
                }
            } else {
                emit_tok(whitespace_tok);
            }
        }
    }

    std::vector<token> local_tokens;
    tokenize(out, local_tokens);
    std::vector<char> local_buf;
    preprocess(local_tokens, local_buf);
    out = local_buf;
    return true;
}

inline bool is_macro_already_expanding(const std::string& name) {
    for(auto& e : expansion_stack) {
        if(name == e) {
            return true;
        }
    }
    return false;
}

inline bool preprocess(const std::vector<token>& tokens, std::vector<char>& out_buf) {
    token tok = tokens[0];
    size_t cur = 0;
    enum {
        PP_DEFAULT, PP_DIRECTIVE,
        PP_INCLUDE, PP_DEFINE, PP_UNDEF, PP_LINE, PP_ERROR, PP_PRAGMA,
        PP_IF, PP_IFDEF, PP_IFNDEF, PP_ELIF, PP_ELSE, PP_ENDIF
    } pp_state = PP_DEFAULT;
    bool fresh_line = true;

    auto advance = [&tokens, &tok, &cur](){
        if(cur == tokens.size() - 1) { tok.type = tok_eof;}
        else { tok = tokens[++cur]; }
    };
    auto go_back = [&tokens, &tok, &cur](int count){
        cur -= count;
        if(cur >= tokens.size()) { tok.type = tok_eof; }
        else { tok = tokens[cur]; }
    };
    auto emit_token_and_advance = [&advance, &out_buf, &tokens, &tok, &cur](){
        out_buf.insert(out_buf.end(), tok.string, tok.string + tok.length);
        advance();
    };
    auto emit_string = [&out_buf](const std::string& str){
        out_buf.insert(out_buf.end(), str.begin(), str.end());
    };
    auto emit_char_array = [&out_buf](const std::vector<char>& buf){
        out_buf.insert(out_buf.end(), buf.begin(), buf.end());
    };
    auto emit_tokens = [&out_buf](const std::vector<token>& tokens){
        for(int i = 0; i < tokens.size(); ++i) {
            auto& tok = tokens[i];
            out_buf.insert(out_buf.end(), tok.string, tok.string + tok.length);
        }
    };
    auto is_tok = [&tok](token_type type) -> bool {
        return tok.type == type;
    };
    auto eat_whitespace = [&advance, &is_tok, &tokens, &tok, &cur](){
        while(is_tok(tok_whitespace)) {
            advance();
        }
    };
    auto eat_whitespace_and_newline = [&advance, &is_tok, &tokens, &tok, &cur]()->int{
        int count = 0;
        while(is_tok(tok_whitespace) || is_tok(tok_newline)) {
            advance();
            ++count;
        }
        return count;
    };

    while(tok.type != tok_eof) {
        switch(pp_state) {
        case PP_DEFAULT:
            if(is_tok(tok_newline)) {
                fresh_line = true;
                emit_token_and_advance();
                continue;
            } else if(is_tok(tok_hash) && fresh_line) {
                pp_state = PP_DIRECTIVE;
                advance();
                continue;
            } else if(is_tok(tok_identifier)) {
                auto it = macros.find(tok.get_string());
                if(it != macros.end() && !is_macro_already_expanding(it->second.name)) {
                    advance(); 
                    // revert_count is used to move cursor back if no opening parenthesis was found
                    int revert_count = eat_whitespace_and_newline();
                    if(it->second.has_parameter_list && is_tok(tok_paren_l)) {
                        advance(); eat_whitespace_and_newline();
                        std::vector<std::vector<token>> macro_args;
                        std::vector<token> va_args;
                        std::vector<token> arg;
                        int open_paren_count = 1;
                        if(is_tok(tok_paren_r)) {
                            advance();
                            continue;
                        } else {
                            while(true) {
                                if(is_tok(tok_eof)) {
                                    pp_error("macro invocation missing closing parenthesis (reached eof)");
                                    return false;
                                }
                                if(is_tok(tok_paren_r)) {
                                    open_paren_count--;
                                    if(open_paren_count == 0) {
                                        if(macro_args.size() < it->second.parameters.size()) {
                                            macro_args.push_back(arg);
                                            advance();
                                        } else {
                                            va_args = arg;
                                            advance();
                                        }
                                        break;
                                    }
                                }
                                if(open_paren_count == 1 && is_tok(tok_comma)) {
                                    if(macro_args.size() < it->second.parameters.size()) {
                                        macro_args.push_back(arg);
                                        arg.clear();
                                        advance(); eat_whitespace_and_newline();
                                    } else {
                                        arg.push_back(tok);
                                        advance();
                                    }
                                    continue;
                                }
                                if(is_tok(tok_paren_l)) {
                                    open_paren_count++;
                                }
                                arg.push_back(tok);

                                advance();
                            }
                        }
                        if(!macro_args.empty()) {
                            for(auto& arg : macro_args) {
                                while(!arg.empty() && arg.back().type == tok_whitespace && arg.back().type == tok_newline) {
                                    arg.pop_back();
                                }
                            }
                        }
                        std::vector<char> replacement;
                        expansion_stack.push_back(it->second.name);
                        if(!process_replacement_list(it->second, replacement, macro_args, va_args)) {
                            return false;
                        }
                        expansion_stack.pop_back();
                        emit_char_array(replacement);
                        fresh_line = false;
                        continue;
                    } else if(it->second.has_parameter_list) {
                        // Macro invocation missing argument list
                        // treat as a non-macro identifier
                        go_back(revert_count);
                        fresh_line = false;
                        //emit_token_and_advance();
                        continue;
                    } else {
                        // Object-like macro invocation
                        std::vector<char> replacement;
                        expansion_stack.push_back(it->second.name);
                        if(!process_replacement_list(it->second, replacement)) {
                            return false;
                        }
                        expansion_stack.pop_back();
                        emit_char_array(replacement);
                        fresh_line = false;
                        go_back(revert_count);
                        continue;
                    }
                } else {
                    // Not a macro identifier
                    fresh_line = false;
                    emit_token_and_advance();
                    continue;
                }
            }
            fresh_line = false;
            emit_token_and_advance();
            break;
        case PP_DIRECTIVE:
            eat_whitespace();
            if(is_tok(tok_newline)) {
                pp_state = PP_DEFAULT;
                break;
            }
            if(!is_tok(tok_identifier)) {
                pp_error("expected an identifier, got '%s'", tok.get_string().c_str());
                return false;
            }
            if(tok_name_match(tok, "define")) {
                pp_state = PP_DEFINE;
                advance();
                break;
            } else {
                while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                    print_tok(tok);
                    advance();
                }
                printf("\n");
                pp_state = PP_DEFAULT;
            }
            break;
        case PP_DEFINE: {
            eat_whitespace();
            if(!is_tok(tok_identifier)) {
                pp_error("expected an identifier, got '%s'", tok.get_string().c_str());
                return false;
            }
            auto& def = macros[tok.get_string()];
            def.name = tok.get_string();
            advance();
            if(is_tok(tok_paren_l)) {
                advance(); eat_whitespace();
                def.has_parameter_list = true;
                while(!is_tok(tok_paren_r)) {
                    if(!is_tok(tok_identifier) && !is_tok(tok_elipsis)) {
                        pp_error("expected an identifier, got '%s'", tok.get_string().c_str());
                        return false;
                    }
                    if(is_tok(tok_elipsis)) {
                        advance(); eat_whitespace();
                        def.has_variadic_param = true;
                        break;
                    }
                    def.parameters.push_back(tok);
                    advance(); eat_whitespace();
                    if(is_tok(tok_comma)) {
                        advance(); eat_whitespace();
                        continue;
                    } else {
                        break;
                    }
                }
                if(!is_tok(tok_paren_r)) {
                    pp_error("expected ')'");
                    return false;
                }
                advance();
            }
            eat_whitespace();
            if(is_tok(tok_double_hash)) {
                pp_error("macro replacement list can't start with '##'");
                return false;
            }
            bool last_token_is_double_hash = false;
            while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                def.replacement_list.push_back(tok);
                if(is_tok(tok_double_hash)) {
                    last_token_is_double_hash = true;
                } else if(!is_tok(tok_whitespace)) {
                    last_token_is_double_hash = false;
                }
                advance();
            }
            if(last_token_is_double_hash) {
                pp_error("macro replacement list can't end with '##'");
                return false;
            }
            pp_state = PP_DEFAULT;
            break;
        }
        case PP_UNDEF:
            eat_whitespace();
            if(!is_tok(tok_identifier)) {
                pp_error("expected an identifier, got '%s'", tok.get_string().c_str());
                return false;
            }
            macros.erase(tok.get_string());
            advance();
            bool has_unexpected_tokens = false;
            while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                has_unexpected_tokens = true;
                advance();
            }
            if(has_unexpected_tokens) {
                // TODO: Warning
            }
            pp_state = PP_DEFAULT;
            break;
        }
    }
    return true;
}

// Replaces all preprocessor directives with blank space
inline void remove_pp(char* buf, size_t len, std::vector<char>& out) {
    
    std::vector<pp_line> lines;

    pp_line l;
    l.buf = buf;
    l.len = 0;
    size_t cur = 0;
    size_t cur_start = 0;
    while(cur < len) {
        char c = buf[cur];
        
        if(c == '\n') {
            l.len = cur - cur_start;
            lines.push_back(l);
            l.buf = buf + cur;
            l.len = 0;
            
            cur_start = cur;
            cur++;
        } else {
            cur++;
        }
    }

    std::map<std::string, std::string> pp_definitions;

    for(auto& l : lines) {
        //printf("line: %s\n", std::string(l.buf, l.len).c_str());
        bool is_pp_line = false;
        size_t cur = 0;
        while(cur < l.len) {
            char c = l.buf[cur];
            if(isspace(c)) {
                ++cur;
                continue;
            } else if(ishash(c)) {
                is_pp_line = true;
                break;
            } else {
                is_pp_line = false;
                break;
            }
        }
        if(!is_pp_line) {
            out.insert(out.end(), l.buf, l.buf + l.len);
        } else {
            // Execute preprocessor directive
        }
    }
}


#endif
