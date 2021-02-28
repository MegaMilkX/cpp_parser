#ifndef CPP_LOG_HPP
#define CPP_LOG_HPP

#include <stdio.h>
#include <string>

namespace cppi {

enum LOG_TYPE {
    LOG_TYPE_MSG,
    LOG_TYPE_WARN,
    LOG_TYPE_ERR
};
typedef void(*log_callback_fn)(LOG_TYPE, const char*);

void set_log_callback(log_callback_fn fn);

}




#endif
