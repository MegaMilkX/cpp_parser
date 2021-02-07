#ifndef TOKENIZE_HPP
#define TOKENIZE_HPP

#include <vector>
#include <assert.h>
#include "token.hpp"

// Tokenize buffer for preprocessing (include whitespace and newline)
bool tokenize(const std::vector<char>& buffer, std::vector<token>& tokens) {
    enum tokenizer_state {
        tstate_default,
        tstate_whitespace,
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
    auto advance = [&c, &buffer, &cid, &column](){
        ++cid;
        if (cid >= buffer.size()) {
            c = '\0';
        } else {
            c = buffer[cid];
            ++column;
        }
    };
    auto submit_token = [&tokens, &tok, &buffer, &cid, &cid_start, &column_start, &column, &line](token_type type){
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
            if(isspace(c)) {
                tstate = tstate_whitespace;
            } else if(isnewline(c)) {
                advance();
                submit_token(tok_newline);
                line++;
                column = 0;
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
            else if(c == '#') { 
                advance();
                if(c == '#') {
                    advance();
                    submit_token(tok_double_hash);
                } else {
                    submit_token(tok_hash);
                } 
            } else if(c == '\"') { tstate = tstate_string_constant; }
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
        case tstate_whitespace:
            advance();
            if(!isspace(c)) {
                submit_token(tok_whitespace);
                tstate = tstate_default;
            }
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
    
    {
        token tok;
        tok.type = tok_eof;
        tokens.push_back(tok);
    }
    return true;
}

#endif
