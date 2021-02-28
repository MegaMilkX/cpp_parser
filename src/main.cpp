
#include <string>
#include "cppi/cppi.hpp"

#include <windows.h>

void log_msg(cppi::LOG_TYPE type, const char* line) {
    printf(line);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        return 1;
    }
    std::string fname = argv[1];

    cppi::set_log_callback(&log_msg);

    cppi::context ctx;
    if(!ctx.parse(fname.c_str())) {
        return 1;
    }
    return 0;
}
