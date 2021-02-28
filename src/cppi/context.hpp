#ifndef CPP_INSPECTOR_CONTEXT_HPP
#define CPP_INSPECTOR_CONTEXT_HPP

#include "pp_context.hpp"
#include "options.hpp"


namespace cppi {

class context {
    pp_context pp_ctx;
public:
    pp_context& get_preprocessor_context() { return pp_ctx; }

    bool parse(const char* fname);
    bool parse(const char* buffer, size_t length, const char* full_file_name_hint = ".");
};

} // cppi


#endif
