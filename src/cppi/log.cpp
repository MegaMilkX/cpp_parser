#include "log.hpp"

#include <assert.h>


namespace cppi {

static log_callback_fn log_msg_cb = 0;

void log_line(LOG_TYPE type, const char* format, ...) {
    constexpr int MAX_LOG_LEN = 1024;
    
    va_list _ArgList;
    __crt_va_start(_ArgList, format);
    char buf[MAX_LOG_LEN];
    int len = sprintf_s(buf, format, _ArgList);
    __crt_va_end(_ArgList);

    assert(len < MAX_LOG_LEN);
    buf[len] = '\n';

    if(log_msg_cb) {
        if(type == LOG_TYPE_MSG) {
            log_msg_cb(type, buf);
        } else if(type == LOG_TYPE_WARN) {
            log_msg_cb(type, (std::string("warning: ") + buf).c_str());
        } else if(type == LOG_TYPE_ERR) {
            log_msg_cb(type, (std::string("error: ") + buf).c_str());
        }
    }
}

void set_log_callback(log_callback_fn fn) {
    log_msg_cb = fn;
}

}