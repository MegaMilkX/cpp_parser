#include "pp_constant_expression.hpp"
#include "pp_token_cursor.hpp"

#include "pp_ast.hpp"
#include "tokenize.hpp"

namespace cppi {

int pp_try_integer_literal(pp_token_cursor& cur) {
    if(cur.is_tok(tok_int_literal)) {
        return 1;
    }
    return 0;
}
int pp_try_character_literal(pp_token_cursor& cur) {
    if(cur.is_tok(tok_char_constant)) {
        return 1;
    }
    return 0;
}
int pp_try_floating_literal(pp_token_cursor& cur) {
    if(cur.is_tok(tok_float_literal)) {
        return 1;
    }
    return 0;
}
int pp_try_string_literal(pp_token_cursor& cur) {
    if(cur.is_tok(tok_string_constant)) {
        return 1;
    }
    return 0;
}
int pp_try_boolean_literal(pp_token_cursor& cur) {
    if(cur.is_identifier("true")) {
        return 1;
    } else if(cur.is_identifier("false")) {
        return 1;
    }
    return 0;
}
int pp_try_pointer_literal(pp_token_cursor& cur) {
    if(cur.is_identifier("nullptr")) {
        return 1;
    }
    return 0;
}
int pp_try_user_defined_literal(pp_token_cursor& cur) {
    return 0;
}
int pp_try_literal(pp_token_cursor cur, ast_node& node) {
    int r = pp_try_integer_literal(cur);
    if(r) {
        node.type = ast_lit_int;
        node.eval_type = ast_lit_int;
        node.as_int = cur.get_tok()->to_int();
        return r;
    }
    r = pp_try_character_literal(cur);
    if(r) {
        node.type = ast_lit_char;
        node.eval_type = ast_lit_char;
        node.as_char = cur.get_tok()->to_char();
        return r;
    }
    r = pp_try_floating_literal(cur);
    if(r) {
        node.type = ast_lit_float;
        node.eval_type = ast_lit_float;
        node.as_float = cur.get_tok()->to_float();
        return r;
    }
    r = pp_try_string_literal(cur);
    if(r) {
        node.type = ast_lit_string;
        node.eval_type = ast_lit_string;
        // TODO cur.get_tok()->to_string() ?
        return r;
    }
    r = pp_try_boolean_literal(cur);
    if(r) {
        node.type = ast_lit_bool;
        node.eval_type = ast_lit_bool;
        node.as_bool = cur.get_tok()->to_bool();
        return r;
    }
    r = pp_try_pointer_literal(cur);
    if(r) {
        node.type = ast_lit_ptr;
        node.eval_type = ast_lit_ptr;
        node.as_int = 0;
        return r;
    }
    r = pp_try_user_defined_literal(cur);
    node.type = ast_unk; // TODO ?
    return r;
}
int pp_try_constant_expression(pp_token_cursor cur, ast_node& node);
int pp_try_primary_expression(pp_token_cursor cur, ast_node& node) {
    int r = pp_try_literal(cur, node);
    if(r) {
        return r;
    }
    int adv = 0;
    if(cur.is_tok(tok_paren_l)) {
        cur.advance(); adv++;
        r = pp_try_constant_expression(cur, node);
        cur.advance(r); adv += r;
        if(!cur.is_tok(tok_paren_r)) {
            return 0;
        }
        cur.advance(); adv++;
        return adv;
    }
    return 0;
}

int pp_try_unary_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    int r = pp_try_primary_expression(cur, node);
    if(r) return r;

    if(cur.is_tok(tok_incr)) {
        // TODO: err
        return 0;
    } else if(cur.is_tok(tok_decr)) {
        // TODO: err
        return 0;
    } else if(cur.is_tok(tok_asterisk)) {
        // TODO: err
        return 0;
    } else if(cur.is_tok(tok_amp)) {
        // TODO: err
        return 0;
    } else if(cur.is_tok(tok_plus)) {
        node.type = ast_unary_plus;
    } else if(cur.is_tok(tok_minus)) {
        node.type = ast_unary_minus;
    } else if(cur.is_tok(tok_excl)) {
        node.type = ast_unary_logic_not;
    } else if(cur.is_tok(tok_tilde)) {
        node.type = ast_unary_not;
    } else {
        return 0;
    }
    cur.advance(); adv++;
    ast_node operand;
    r = pp_try_unary_expression(cur, operand);
    cur.advance(r); adv += r;
    if(!r) {
        return 0;
    }
    node.set_right(operand);
    return adv;
}

int pp_try_multiplicative_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_unary_expression(cur, node_a);
    cur.advance(r); adv += r;
    if(!r) return 0;
    if(cur.is_tok(tok_asterisk)) {
        cur.advance(); adv++;
        node.type = ast_mul;
    } else if(cur.is_tok(tok_fwd_slash)) {
        cur.advance(); adv++;
        node.type = ast_div;
    } else if(cur.is_tok(tok_percent)) {
        cur.advance(); adv++;
        node.type = ast_div_rem;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_multiplicative_expression(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_additive_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_multiplicative_expression(cur, node_a);
    cur.advance(r); adv += r;
    if(!r) return 0;
    if(cur.is_tok(tok_plus)) {
        cur.advance(); adv++;
        node.type = ast_plus;
    } else if(cur.is_tok(tok_minus)) {
        cur.advance(); adv++;
        node.type = ast_minus;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_additive_expression(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_shift_expession(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_additive_expression(cur, node_a);
    cur.advance(r); adv += r;
    if(cur.is_tok(tok_shift_left)) {
        cur.advance(); adv++;
        node.type = ast_shift_left;
    } else if(cur.is_tok(tok_shift_right)) {
        cur.advance(); adv++;
        node.type = ast_shift_right;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_shift_expession(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_relational_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_shift_expession(cur, node_a);
    cur.advance(r); adv += r;
    if(cur.is_tok(tok_less)) {
        cur.advance(); adv++;
        node.type = ast_less;
    } else if(cur.is_tok(tok_more)) {
        cur.advance(); adv++;
        node.type = ast_more;
    } else if(cur.is_tok(tok_less_assign)) {
        cur.advance(); adv++;
        node.type = ast_less_eq;
    } else if(cur.is_tok(tok_more_assign)) {
        cur.advance(); adv++;
        node.type = ast_more_eq;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_relational_expression(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_equality_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_relational_expression(cur, node_a);
    cur.advance(r); adv += r;
    if(cur.is_tok(tok_equals)) {
        cur.advance(); adv++;
        node.type = ast_equal;
    } else if(cur.is_tok(tok_excl_assign)) {
        cur.advance(); adv++;
        node.type = ast_not_equal;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_equality_expression(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_and_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_equality_expression(cur, node_a);
    cur.advance(r); adv += r;
    if(cur.is_tok(tok_amp)) {
        cur.advance(); adv++;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_and_expression(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.type = ast_and;
    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_exclusive_or_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_and_expression(cur, node_a);
    cur.advance(r); adv += r;
    if(cur.is_tok(tok_hat)) {
        cur.advance(); adv++;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_exclusive_or_expression(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.type = ast_excl_or;
    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_inclusive_or_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_exclusive_or_expression(cur, node_a);
    cur.advance(r); adv += r;
    if(cur.is_tok(tok_pipe)) {
        cur.advance(); adv++;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_inclusive_or_expression(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.type = ast_or;
    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_logical_and_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_inclusive_or_expression(cur, node_a);
    cur.advance(r); adv += r;
    if(cur.is_tok(tok_double_amp)) {
        cur.advance(); adv++;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_logical_and_expression(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.type = ast_logic_and;
    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_logical_or_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_logical_and_expression(cur, node_a);
    cur.advance(r); adv+=r;
    if(cur.is_tok(tok_double_pipe)) {
        cur.advance(); adv++;
    } else {
        node = node_a;
        return adv;
    }
    ast_node node_b;
    r = pp_try_logical_or_expression(cur, node_b);
    cur.advance(r); adv += r;
    if(!r) return 0;

    node.type = ast_logic_or;
    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}
int pp_try_conditional_expression(pp_token_cursor cur, ast_node& node) {
    int adv = 0;
    ast_node node_a;
    int r = pp_try_logical_or_expression(cur, node_a);
    cur.advance(r); adv += r;
    if(cur.is_tok(tok_question)) {
        cur.advance(); adv++;
    } else {
        node = node_a;
        return adv;
    }

    ast_node node_b;
    ast_node node_b_a;
    r = pp_try_constant_expression(cur, node_b_a); // TODO 'expression', not 'constant-expression'
    cur.advance(r); adv += r;
    if(!r) return 0;
    if(!cur.is_tok(tok_colon)) {
        return 0;
    }
    cur.advance(); adv++;
    
    ast_node node_b_b;
    r = pp_try_conditional_expression(cur, node_b_b); // TODO 'assignment-expression', not 'conditional-expression'
    cur.advance(r); adv += r;
    if(!r) return 0;

    node_b.type = ast_conditional_options;
    node_b.set_left(node_b_a);
    node_b.set_right(node_b_b);

    node.type = ast_conditional;
    node.set_left(node_a);
    node.set_right(node_b);

    return adv;
}

int pp_try_constant_expression(pp_token_cursor cur, ast_node& node) {
    int r = pp_try_conditional_expression(cur, node);
    return r;   
}

} // cppi