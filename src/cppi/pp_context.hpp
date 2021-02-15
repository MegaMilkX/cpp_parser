#ifndef CPP_INSPECTOR_PREPROCESSOR_CONTEXT_HPP
#define CPP_INSPECTOR_PREPROCESSOR_CONTEXT_HPP

#include <string>
#include <map>
#include <vector>

#include "token.hpp"


namespace cppi {

class pp_context {
    std::vector<char> preprocessed_buffer;

    struct pp_macro {
        std::string name;
        bool has_parameter_list = false;
        bool has_variadic_param = false;
        std::vector<token> parameters;
        std::vector<token> replacement_list;
    };
    std::map<std::string, pp_macro> macros;
    std::vector<std::string> expansion_stack; // to track circular expansion

    enum CONDITION_TYPE {
        COND_IF,
        COND_ELIF,
        COND_ELSE
    };
    struct pp_cond_state {
        CONDITION_TYPE  type;
        bool            group_enabled;
        bool            one_condition_already_satisfied;
    };
    std::vector<pp_cond_state> conditional_stack; // #ifdef, etc. 0 - false, otherwise - true
    bool pp_token_group_enabled = true;

    void pp_error(const char* format, ...);
    bool is_macro_already_expanding(const std::string& name);
    bool pp_eval_constant_expression(const std::vector<token>& tokens, int& out);
    std::vector<char> pp_string_constant_from_tokens(const std::vector<token>& tokens);
    std::vector<char> pp_string_from_tokens(const std::vector<token>& tokens);
    bool process_replacement_list(
        const pp_macro& macro,
        std::vector<char>& out,
        const std::vector<std::vector<token>>& macro_args = std::vector<std::vector<token>>(),
        const std::vector<token>& va_args = std::vector<token>()
    );
    bool preprocess(
        const std::vector<token>& tokens, 
        std::vector<char>& out_buf,
        const std::string& full_fpath = "", 
        bool constant_expression = false, 
        bool include_line = false
    );

public:
    bool preprocess(const char* buffer, size_t length, const char* full_file_path_hint = 0);

    size_t get_preprocessed_length() const;
    const char* get_preprocessed_buffer() const;

};

} // cppi


#endif
