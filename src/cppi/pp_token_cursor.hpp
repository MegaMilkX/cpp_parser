#ifndef CPPI_PP_TOKEN_CURSOR_HPP
#define CPPI_PP_TOKEN_CURSOR_HPP

#include <string>
#include <vector>
#include "token.hpp"


namespace cppi {

class pp_token_cursor {
    const token* tokens;
    const token* tok;
    size_t count;
    size_t cur;
public:
    pp_token_cursor(const std::vector<token>& tokens) {
        this->tokens = tokens.data();
        count = tokens.size();
        cur = 0;
        tok = &tokens[cur];
    }
    void advance(int i = 1) {
        cur += i;
        if(cur >= count) {
            tok = 0;
        } else {
            tok = &tokens[cur];
        }
    }
    bool is_tok(token_type type) {
        if(!tok) {
            if(type == tok_eof) {
                return true;
            } else {
                return false;
            }
        }
        return tok->type == type;
    }
    bool is_identifier(const std::string& comp) {
        if(!is_tok(tok_identifier)) {
            return false;
        }
        return tok_name_match(*tok, comp.c_str());
    }
    const token* get_tok() const {
        return tok;
    }
};

} // cppi


#endif
