#ifndef CPPI_LOG_INTERNAL_HPP
#define CPPI_LOG_INTERNAL_HPP

#include "log.hpp"

namespace cppi {

void log_line(LOG_TYPE type, const char* format, ...);

}

#define LOG(FORMAT, ...) log_line(LOG_TYPE_MSG, FORMAT, __VA_ARGS__)
#define LOG_WARN(FORMAT, ...) log_line(LOG_TYPE_WARN, FORMAT, __VA_ARGS__)
#define LOG_ERR(FORMAT, ...) log_line(LOG_TYPE_ERR, FORMAT, __VA_ARGS__)


#endif
