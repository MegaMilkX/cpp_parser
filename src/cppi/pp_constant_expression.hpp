#ifndef CPPI_PREPROCESSOR_CONSTANT_EXPRESSION_HPP
#define CPPI_PREPROCESSOR_CONSTANT_EXPRESSION_HPP

#include <vector>
#include "pp_token_cursor.hpp"
#include "pp_ast.hpp"


namespace cppi {

int  pp_try_constant_expression(pp_token_cursor cur, ast_node& node = ast_node());
bool pp_eval_constant_expression(const std::vector<token>& tokens, int& out);

}


#endif
