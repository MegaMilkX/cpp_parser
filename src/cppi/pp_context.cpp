#include "pp_context.hpp"

#include <cctype>
#include "tokenize.hpp"
#include "pp_constant_expression.hpp"
#include "file_util.hpp"
#include "pp_token_cursor.hpp"
#include "pp_ast.hpp"


namespace cppi {

void print_tok(const token& tok) {
    printf("%s", tok.get_string().c_str());
}

void pp_context::pp_error(const char* format, ...) {
    va_list _ArgList;
    __crt_va_start(_ArgList, format);
    printf((std::string("PP ERROR: ") + format).c_str(), _ArgList);
    __crt_va_end(_ArgList);
}

std::string pp_dir_name_from_path(const std::string& path) {
    if(path == ".") {
        return ".";
    }
    std::string _path = path;
    std::replace_if(_path.begin(), _path.end(), [](char c)->bool{ return c == '/'; }, '\\');
    std::transform(_path.begin(), _path.end(), _path.begin(), [](char c)->char{
        return std::tolower(c);
    });
    size_t last_slash = _path.find_last_of('\\');
    assert(last_slash > 0);
    return std::string(_path.data(), _path.data() + last_slash + 1);
}

bool pp_context::is_macro_already_expanding(const std::string& name) {
    for(auto& e : expansion_stack) {
        if(name == e) {
            return true;
        }
    }
    return false;
}

bool pp_context::pp_eval_constant_expression(const std::vector<token>& tokens, int& out) {
    std::vector<char> preprocessed_expr;
    if(!preprocess(tokens, preprocessed_expr, "", true)) {
        return false;
    }
    std::vector<token> preprocessed_tokens;
    tokenize(preprocessed_expr, preprocessed_tokens);
    
    std::vector<token> tokens_no_whitespace;
    for(int i = 0; i < preprocessed_tokens.size(); ++i) {
        if(preprocessed_tokens[i].type == tok_whitespace || preprocessed_tokens[i].type == tok_newline) {
            continue;
        }
        tokens_no_whitespace.push_back(preprocessed_tokens[i]);
    }

    pp_token_cursor cur(tokens_no_whitespace);
    
    ast_node node;
    int r = pp_try_constant_expression(cur, node);
    for(int i = 0; i < r; ++i) {
        printf("%s ", tokens_no_whitespace[i].get_string().c_str());
    }
    printf("\n");
    printf("result: ");
    node.eval();
    if(node.eval_type == ast_lit_int) { printf("%i", node.as_int); }
    else if(node.eval_type == ast_lit_char) { printf("%c", node.as_char); }
    else if(node.eval_type == ast_lit_float) { printf("%f", node.as_float); }
    else if(node.eval_type == ast_lit_bool) { node.as_bool ? printf("true") : printf("false"); }
    else { assert(false); }
    printf("\n");

    if(node.eval_type == ast_lit_int) { out = node.as_int; }
    else if(node.eval_type == ast_lit_char) { out = node.as_char; }
    else if(node.eval_type == ast_lit_float) { out = node.as_float; }
    else if(node.eval_type == ast_lit_bool) { out = node.as_bool; }
    else { return false; }
    return true;
}

std::vector<char> pp_context::pp_string_constant_from_tokens(const std::vector<token>& tokens) {
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
    if(out.back() == ' ') {
        out.pop_back();
    }
    out.push_back('"');

    return out;
}

std::vector<char> pp_context::pp_string_from_tokens(const std::vector<token>& tokens) {
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

bool pp_context::process_replacement_list(const pp_macro& macro, std::vector<char>& out, const std::vector<std::vector<token>>& macro_args, const std::vector<token>& va_args) {
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
                if(tok.get_string() == "__VA_ARGS__") {
                    std::vector<char> str = pp_string_constant_from_tokens(va_args);
                    emit_buf(str);
                    advance();
                } else {
                    pp_error("token after # must be a macro parameter");
                    return false;
                }
            } else {
                std::vector<char> stringified_arg;
                if(param_idx >= macro_args.size()) {
                    // TODO: WARNING: Not enough parameters
                    stringified_arg = { '"', '"' };
                } else {
                    stringified_arg = pp_string_constant_from_tokens(macro_args[param_idx]);                
                }
                emit_buf(stringified_arg);
                advance();
            }
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

bool pp_context::preprocess(
    const std::vector<token>& tokens, 
    std::vector<char>& out_buf,
    const std::string& full_fpath, 
    bool constant_expression, 
    bool include_line
) {
    std::string full_file_path = full_fpath;
    if(full_file_path.empty()) {
        full_file_path = ".";
    }
    bool ignore_directives = constant_expression || include_line;

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
    auto emit_token_and_advance = [this, &advance, &out_buf, &tokens, &tok, &cur](){
        if(pp_token_group_enabled) {
            out_buf.insert(out_buf.end(), tok.string, tok.string + tok.length);
        }
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
    auto is_group_enabled = [this]()->bool{
        if(conditional_stack.empty()) {
            return true;
        }
        bool parent_state;
        if(conditional_stack.size() == 1) {
            parent_state = true;
        } else {
            parent_state = conditional_stack[conditional_stack.size() - 2].group_enabled;
        }
        return conditional_stack.back().group_enabled && parent_state;
    };
    auto is_parent_group_enabled = [this]()->bool{
        if(conditional_stack.empty()) {
            return true;
        }
        bool parent_state;
        if(conditional_stack.size() == 1) {
            parent_state = true;
        } else {
            parent_state = conditional_stack[conditional_stack.size() - 2].group_enabled;
        }
        return parent_state;
    };

    while(tok.type != tok_eof) {
        switch(pp_state) {
        case PP_DEFAULT:
            if(is_tok(tok_newline)) {
                fresh_line = true;
                emit_token_and_advance();
                continue;
            } else if(is_tok(tok_hash) && fresh_line) {
                if(ignore_directives) {
                    pp_error("# is unexprected in a constant expression");
                    return false;
                }
                pp_state = PP_DIRECTIVE;
                advance();
                continue;
            } else if(is_tok(tok_identifier)) {
                if(constant_expression && tok_name_match(tok, "defined")) {
                    advance();
                    eat_whitespace();
                    bool parenthesized = false;
                    if(is_tok(tok_paren_l)) {
                        parenthesized = true;
                        advance();
                        eat_whitespace();
                    }
                    if(!is_tok(tok_identifier)) {
                        pp_error("expected an identifier");
                        return false;
                    }
                    bool is_defined = macros.find(tok.get_string()) != macros.end();
                    if(parenthesized) {
                        advance();
                        eat_whitespace();
                        if(!is_tok(tok_paren_r)) {
                            pp_error("exprected closing parenthesis");
                            return false;
                        }
                    }
                    if(is_defined) {
                        emit_string("1");
                    } else {
                        emit_string("0");
                    }
                    advance();
                    continue;
                }
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
                        if(constant_expression) {
                            emit_string("0");
                            advance();
                        } else {
                            emit_string(it->second.name);
                            advance();
                        }
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
                    if(!constant_expression) {
                        emit_token_and_advance();
                    } else {
                        emit_string("0");
                        advance();
                    }
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
            if(pp_token_group_enabled && tok_name_match(tok, "include")) {
                pp_state = PP_INCLUDE;
                advance();
            } else if(pp_token_group_enabled && tok_name_match(tok, "define")) {
                pp_state = PP_DEFINE;
                advance();
                break;
            } else if(pp_token_group_enabled && tok_name_match(tok, "undef")) {
                pp_state = PP_UNDEF;
                advance();
                break;
            } else if(tok_name_match(tok, "if")) {
                pp_state = PP_IF;
                advance();
                break;
            } else if(tok_name_match(tok, "ifdef")) {
                pp_state = PP_IFDEF;
                advance();
                break;
            } else if(tok_name_match(tok, "ifndef")) {
                pp_state = PP_IFNDEF;
                advance();
                break;
            } else if(tok_name_match(tok, "else")) {
                pp_state = PP_ELSE;
                advance();
                break;
            } else if(tok_name_match(tok, "elif")) {
                pp_state = PP_ELIF;
                advance();
                break;
            } else if(tok_name_match(tok, "endif")) {
                pp_state = PP_ENDIF;
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
        case PP_INCLUDE: {
            eat_whitespace();
            std::vector<token> incl_tokens;
            while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                incl_tokens.push_back(tok);
                advance();
            }
            std::vector<char> buf;
            if(!preprocess(incl_tokens, buf, "", false, true)) {
                pp_error("failed to preprocess tokens after #include");
                return false;
            }
            if(buf.empty()) {
                pp_error("missing file path for #include");
                return false;
            }
            bool is_quotes;
            if(buf[0] == '\"') {
                is_quotes = true;
            } else if(buf[0] == '<') {
                is_quotes = false;
            } else {
                pp_error("expected \" or < for #include");
                return false;
            }
            std::string fname;
            int fname_len = 0;
            for(int i = 1; i < buf.size(); ++i) {
                if(is_quotes && buf[i] == '\"') {
                    break;
                } else if(!is_quotes && buf[i] == '>') {
                    break;
                }
                ++fname_len;
            }
            if(fname_len == buf.size() - 1) {
                pp_error("missing closing \" or > for #include");
                return false;
            }
            fname = std::string(buf.data() + 1, buf.data() + 1 + fname_len);
            if(fname.empty()) {
                pp_error("file name required for #include");
                return false;
            }

            if(is_quotes) {
                std::string dir = pp_dir_name_from_path(full_file_path);
                std::string new_fname = dir + "\\" + fname;
                
                std::vector<char> fbuf;
                if(!load_file(new_fname.c_str(), fbuf)) {
                    pp_error("can't find include file '%s'", fname.c_str());
                    return false;
                }

                std::vector<token> _pp_tokens;
                tokenize(fbuf, _pp_tokens);
                std::vector<char> _preprocessed_buffer;
                preprocess(_pp_tokens, _preprocessed_buffer, new_fname);
                emit_char_array(_preprocessed_buffer);
            } else {
                // TODO
            }

            printf("include %s\n", fname.c_str());
            pp_state = PP_DEFAULT;
            break;
        }
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
        case PP_UNDEF: {
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
        case PP_IF: {
            eat_whitespace();
            std::vector<token> expr_tokens;
            while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                expr_tokens.push_back(tok);
                advance();
            }
            int val;
            if(!pp_eval_constant_expression(expr_tokens, val)) {
                pp_error("failed to evaluate constant expression");
                return false;
            }
            bool group_enabled = val != 0;

            pp_cond_state cond_state;
            cond_state.type = COND_IF;
            cond_state.group_enabled = group_enabled && pp_token_group_enabled;
            cond_state.one_condition_already_satisfied = cond_state.group_enabled || !pp_token_group_enabled;
            conditional_stack.push_back(cond_state);
            pp_token_group_enabled = cond_state.group_enabled;

            pp_state = PP_DEFAULT;
            break;
        }
        case PP_IFDEF: {
            eat_whitespace();
            if(!is_tok(tok_identifier)) {
                pp_error("expected an identifier, got '%s'", tok.get_string().c_str());
                return false;
            }
            auto it = macros.find(tok.get_string());
            bool group_enabled = it != macros.end();

            pp_cond_state cond_state;
            cond_state.type = COND_IF;
            cond_state.group_enabled = group_enabled && pp_token_group_enabled;
            cond_state.one_condition_already_satisfied = cond_state.group_enabled || !pp_token_group_enabled;
            conditional_stack.push_back(cond_state);
            pp_token_group_enabled = cond_state.group_enabled;
            while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                advance();
            }
            pp_state = PP_DEFAULT;
            break;
        }
        case PP_IFNDEF: {
            eat_whitespace();
            if(!is_tok(tok_identifier)) {
                pp_error("expected an identifier, got '%s'", tok.get_string().c_str());
                return false;
            }
            auto it = macros.find(tok.get_string());
            bool group_enabled = it == macros.end();

            pp_cond_state cond_state;
            cond_state.type = COND_IF;
            cond_state.group_enabled = group_enabled && pp_token_group_enabled;
            cond_state.one_condition_already_satisfied = cond_state.group_enabled || !pp_token_group_enabled;
            conditional_stack.push_back(cond_state);
            pp_token_group_enabled = cond_state.group_enabled;
            while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                advance();
            }
            pp_state = PP_DEFAULT;
            break;
        }
        case PP_ELSE: {
            if(conditional_stack.empty()) {
                pp_error("unexpected else directive");
                return false;
            }
            auto& cond_state = conditional_stack.back();
            if(cond_state.type != COND_ELIF && cond_state.type != COND_IF) {
                pp_error("unexpected else directive");
                return false;
            }

            cond_state.type = COND_ELSE;
            cond_state.group_enabled = !cond_state.one_condition_already_satisfied && is_parent_group_enabled();
            pp_token_group_enabled = cond_state.group_enabled;

            while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                advance();
            }
            pp_state = PP_DEFAULT;
            break;
        }
        case PP_ELIF: {
            if(conditional_stack.empty()) {
                pp_error("unexpected else directive");
                return false;
            }
            auto& cond_state = conditional_stack.back();
            if(cond_state.type != COND_ELIF && cond_state.type != COND_IF) {
                pp_error("unexpected else directive");
                return false;
            }
            eat_whitespace();
            std::vector<token> expr_tokens;
            while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                expr_tokens.push_back(tok);
                advance();
            }
            int val;
            if(!pp_eval_constant_expression(expr_tokens, val)) {
                pp_error("failed to evaluate constant expression");
                return false;
            }
            bool group_enabled = val != 0;

            cond_state.type = COND_ELIF;
            cond_state.group_enabled = group_enabled && !cond_state.one_condition_already_satisfied && is_parent_group_enabled();
            pp_token_group_enabled = cond_state.group_enabled;

            pp_state = PP_DEFAULT;
            break;
        }
        case PP_ENDIF:
            if(conditional_stack.empty()) {
                pp_error("unexpected endif directive");
                return false;
            }
            conditional_stack.pop_back();
            pp_token_group_enabled = is_parent_group_enabled();
            while(!is_tok(tok_newline) && !is_tok(tok_eof)) {
                advance();
            }
            pp_state = PP_DEFAULT;
            break;
        }
    }
    return true;
}

bool pp_context::preprocess(const char* buffer, size_t length, const char* full_file_path_hint) {
    std::vector<char> buf(buffer, buffer + length);
    std::vector<token> pp_tokens;
    if(!tokenize(buf, pp_tokens)) {
        return false;
    }

    preprocess(pp_tokens, preprocessed_buffer, full_file_path_hint);

    // Debug
    dump_buffer(preprocessed_buffer, (std::string(full_file_path_hint) + ".pp").c_str());

    return true;
}

size_t pp_context::get_preprocessed_length() const {
    return preprocessed_buffer.size();
}
const char* pp_context::get_preprocessed_buffer() const {
    return preprocessed_buffer.data();
}

}