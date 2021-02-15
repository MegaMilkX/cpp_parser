#ifndef CPP_INSPECTOR_CONTEXT_HPP
#define CPP_INSPECTOR_CONTEXT_HPP

#include "pp_context.hpp"


namespace cppi {

class context {
public:
    void parse(const char* fname);
    void parse(const char* buffer, size_t length);
};

} // cppi


#endif
