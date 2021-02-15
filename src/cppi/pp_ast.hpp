#ifndef CPPI_PP_AST_HPP
#define CPPI_PP_AST_HPP

#include <memory>
#include <assert.h>
#include <stdint.h>


namespace cppi {

enum ast_node_type {
    ast_unk,
    ast_lit_int,
    ast_lit_float,
    ast_lit_char,
    ast_lit_string,
    ast_lit_bool,
    ast_lit_ptr,
    ast_plus,
    ast_minus,
    ast_mul,
    ast_div,
    ast_div_rem,
    ast_shift_left,
    ast_shift_right,
    ast_less,
    ast_more,
    ast_less_eq,
    ast_more_eq,
    ast_equal,
    ast_not_equal,
    ast_and,
    ast_excl_or,
    ast_or,
    ast_logic_and,
    ast_logic_or,
    ast_conditional,
    ast_conditional_options,

    ast_unary_plus,
    ast_unary_minus,
    ast_unary_not,
    ast_unary_logic_not
};

enum ast_var_type {
    ast_var_int,
    ast_var_float,
    ast_var_char,
    ast_var_bool
};

class ast_var {
    ast_var_type type;
    union {
        uint64_t blob;
        int     _int;
        float   _float;
        char    _char;
        bool    _bool;
    };
public:
    ast_var(int val)
    : type(ast_var_int), _int(val) {}
    ast_var(float val)
    : type(ast_var_float), _float(val) {}
    ast_var(char val)
    : type(ast_var_char), _char(val) {}
    ast_var(bool val)
    : type(ast_var_bool), _bool(val) {}

    bool is_int() const { return type == ast_var_int; }
    bool is_float() const { return type == ast_var_float; }
    bool is_char() const { return type == ast_var_char; }
    bool is_bool() const { return type == ast_var_bool; }
};

class ast_node {
public:
    ast_node_type type;
    ast_node_type eval_type;
    std::shared_ptr<ast_node> left;
    std::shared_ptr<ast_node> right;

    union {
        uint64_t blob;
        int as_int;
        float as_float;
        char as_char;
        // TODO string
        bool as_bool;
    };

    void set_left(const ast_node& node) {
        left.reset(new ast_node(node));
    }
    void set_right(const ast_node& node) {
        right.reset(new ast_node(node));
    }

    void eval() {
        switch(type) {
        case ast_plus:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a + b;

            break;
        }
        case ast_minus:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a - b;

            break;
        }
        case ast_mul:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a * b;

            break;
        }
        case ast_div:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a / b; // TODO: Check for division by zero

            break;
        }
        case ast_div_rem:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a % b;

            break;
        }
        case ast_shift_left:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a << b;

            break;
        }
        case ast_shift_right:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a >> b;

            break;
        }
        case ast_less:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_bool;
            as_bool = a < b;

            break;
        }
        case ast_more:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_bool;
            as_bool = a > b;

            break;
        }
        case ast_less_eq:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_bool;
            as_bool = a <= b;

            break;
        }
        case ast_more_eq:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_bool;
            as_bool = a >= b;

            break;
        }
        case ast_equal:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_bool;
            as_bool = a == b;

            break;
        }
        case ast_not_equal:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_bool;
            as_bool = a != b;

            break;
        }
        case ast_and:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a & b;

            break;
        }
        case ast_excl_or:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a ^ b;

            break;
        }
        case ast_or:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_int;
            as_int = a | b;

            break;
        }
        case ast_logic_and:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_bool;
            as_bool = a && b;

            break;
        }
        case ast_logic_or:{
            left->eval();
            right->eval();
            int a, b;
            if(left->eval_type == ast_lit_bool) { a = left->as_bool; }
            else if(left->eval_type == ast_lit_char) { a = left->as_char; }
            else if(left->eval_type == ast_lit_float) { a = left->as_float; }
            else { a = left->as_int; }
            if(right->eval_type == ast_lit_bool) { b = right->as_bool; }
            else if(right->eval_type == ast_lit_char) { b = right->as_char; }
            else if(right->eval_type == ast_lit_float) { b = right->as_float; }
            else { b = right->as_int; }
            eval_type = ast_lit_bool;
            as_bool = a || b;

            break;
        }
        case ast_conditional:{
            left->eval();
            bool cond;
            if(left->eval_type == ast_lit_bool) { cond = left->as_bool; }
            else if(left->eval_type == ast_lit_int) { cond = left->as_int; }
            else if(left->eval_type == ast_lit_float) { cond = left->as_float; }
            else if(left->eval_type == ast_lit_char) { cond = left->as_char; }
            else { assert(false); }

            ast_node* conditions = right.get();
            if(cond) {
                conditions->left->eval();
                eval_type = conditions->left->eval_type;
                blob = conditions->left->blob;
            } else {
                conditions->right->eval();
                eval_type = conditions->right->eval_type;
                blob = conditions->right->blob;
            }
            
            break;
        }
        case ast_unary_plus:{
            right->eval();
            int val;
            if(right->eval_type == ast_lit_bool) { val = right->as_bool; }
            else if(right->eval_type == ast_lit_int) { val = right->as_int; }
            else if(right->eval_type == ast_lit_float) { val = right->as_float; }
            else if(right->eval_type == ast_lit_char) { val = right->as_char; }
            else { assert(false); }
            eval_type = ast_lit_int;
            as_int = val;
            break;
        }
        case ast_unary_minus:{
            right->eval();
            int val;
            if(right->eval_type == ast_lit_bool) { val = right->as_bool; }
            else if(right->eval_type == ast_lit_int) { val = right->as_int; }
            else if(right->eval_type == ast_lit_float) { val = right->as_float; }
            else if(right->eval_type == ast_lit_char) { val = right->as_char; }
            else { assert(false); }
            eval_type = ast_lit_int;
            as_int = -val;
            break;
        }
        case ast_unary_not:{
            right->eval();
            int val;
            if(right->eval_type == ast_lit_bool) { val = right->as_bool; }
            else if(right->eval_type == ast_lit_int) { val = right->as_int; }
            else if(right->eval_type == ast_lit_float) { val = right->as_float; }
            else if(right->eval_type == ast_lit_char) { val = right->as_char; }
            else { assert(false); }
            eval_type = ast_lit_int;
            as_int = ~val;
            break;
        }
        case ast_unary_logic_not:{
            right->eval();
            bool val;
            if(right->eval_type == ast_lit_bool) { val = right->as_bool; }
            else if(right->eval_type == ast_lit_int) { val = right->as_int; }
            else if(right->eval_type == ast_lit_float) { val = right->as_float; }
            else if(right->eval_type == ast_lit_char) { val = right->as_char; }
            else { assert(false); }
            eval_type = ast_lit_bool;
            as_int = !val;
            break;
        }
        };
    }
};

} // cppi


#endif
