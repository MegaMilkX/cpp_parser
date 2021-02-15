#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <assert.h>
#include <algorithm>
#include <string>

#include "util.hpp"

enum token_type {
    tok_error,
    tok_identifier,
    tok_string_literal,
    tok_char_literal,
    tok_int_literal,
    tok_float_literal,
    tok_literal,
    tok_paren_l,
    tok_paren_r,
    tok_bracket_l,
    tok_bracket_r,
    tok_double_bracket_l,
    tok_double_bracket_r,
    tok_brace_l,
    tok_brace_r,
    tok_comma,
    tok_dot,
    tok_colon,
    tok_semicolon,
    tok_comment,
    tok_string_constant,
    tok_char_constant,
    tok_hash,
    tok_double_hash,
    
    tok_assign,
    tok_equals,

    tok_plus,
    tok_incr,
    tok_plus_assign,

    tok_minus,
    tok_decr,
    tok_minus_assign,
    tok_arrow,
    tok_arrow_member,

    tok_asterisk,
    tok_asterisk_assign,

    tok_double_colon,
    tok_question,
    tok_elipsis,
    tok_dot_asterisk,
    tok_hat,
    tok_hat_assign,
    tok_tilde,
    tok_pipe,
    tok_pipe_assign,
    tok_double_pipe,
    tok_amp,
    tok_amp_assign,
    tok_double_amp,
    tok_more,
    tok_shift_right,
    tok_shift_right_assign,
    tok_more_assign,
    tok_less,
    tok_shift_left,
    tok_shift_left_assign,
    tok_less_assign,
    tok_three_way_comp,
    tok_excl,
    tok_excl_assign,
    tok_percent,
    tok_percent_assign,
    tok_fwd_slash,
    tok_fwd_slash_assign,

    tok_alignas,
    tok_alignof,
    tok_and,
    tok_and_eq,
    tok_asm,
    tok_atomic_cancel,
    tok_atomic_commit,
    tok_atomic_noexcept,
    tok_bitand,
    tok_bitor,
    tok_break,
    tok_case,
    tok_catch,
    tok_class,
    tok_compl,
    tok_concept,
    tok_const,
    tok_consteval,
    tok_constexpr,
    tok_constinit,
    tok_const_cast,
    tok_continue,
    tok_co_await,
    tok_co_return,
    tok_co_yield,
    tok_decltype,
    tok_default,
    tok_delete,
    tok_do,
    tok_dynamic_cast,
    tok_else,
    tok_enum,
    tok_explicit,
    tok_export,
    tok_extern,
    tok_false,
    tok_for,
    tok_friend,
    tok_goto,
    tok_if,
    tok_inline,
    tok_mutable,
    tok_namespace,
    tok_new,
    tok_noexcept,
    tok_not,
    tok_not_eq,
    tok_nullptr,
    tok_operator,
    tok_or,
    tok_or_eq,
    tok_private,
    tok_protected,
    tok_public,
    tok_reflexpr,
    tok_register,
    tok_reinterpret_cast,
    tok_requires,
    tok_return,
    tok_sizeof,
    tok_static,
    tok_static_assert,
    tok_static_cast,
    tok_struct,
    tok_switch,
    tok_template,
    tok_this,
    tok_thread_local,
    tok_throw,
    tok_true,
    tok_try,
    tok_typedef,
    tok_typeid,
    tok_typename,
    tok_union,
    tok_using,
    tok_virtual,
    tok_volatile,
    tok_while,
    tok_xor,
    tok_xor_eq,

    tok_char,
    tok_char16_t,
    tok_char32_t,
    tok_wchar_t,
    tok_bool,
    tok_short,
    tok_int,
    tok_long,
    tok_signed,
    tok_unsigned,
    tok_float,
    tok_double,
    tok_void,
    tok_auto,

    tok_final,
    tok_override,

    tok_whitespace,
    tok_newline,
    tok_eof
};

struct token;
inline bool tok_name_match(const token& tok, const char* name);

struct token {
    token_type  type = tok_error;
    const char* string;
    size_t      length;

    size_t      line;
    size_t      col;

    std::string get_string() const {
        if (type == tok_eof) {
            return "";
        }
        return std::string(string, length);
    }
    int to_int() const {
        assert(type == tok_int_literal);
        return std::stoi(get_string());
    }
    float to_float() const {
        assert(type == tok_float_literal);
        // TODO Handle literals (f in 10.0f)
        return std::stof(get_string());
    }
    char to_char() const {
        assert(type == tok_char_constant);
        return 0; // TODO
    }
    std::string to_string() const {
        assert(type == tok_string_constant);
        return std::string(string + 1, string + length - 1);
    }
    bool to_bool() const {
        if(tok_name_match(*this, "true")) {
            return true;
        } else if(tok_name_match(*this, "false")) {
            return false;
        } else {
            assert(false);
            return false;
        }
    }
};

inline bool tok_name_match(const token& tok, const char* name) {
    size_t len = strlen(name);
    if(tok.length != len) {
        return false;
    }
    return strncmp(tok.string, name, len) == 0;
}

class token_cursor {
    token* tokens;
    size_t cur;
public:
    token_cursor(token* tokens)
    : tokens(tokens), cur(0) {

    }
    const token& current() const {
        return tokens[cur];
    }
    const token& next() const {
        return tokens[cur + 1];
    }
    void advance() {
        ++cur;
    }
};

#endif