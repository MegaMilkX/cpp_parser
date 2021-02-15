
#include <string>
#include "cppi/cppi.hpp"


int main(int argc, char** argv) {
    if(argc < 2) {
        return 1;
    }
    std::string fname = argv[1];

    cppi::context ctx;
    if(!ctx.parse(fname.c_str())) {
        return 1;
    }
    return 0;
}
