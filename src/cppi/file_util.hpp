#ifndef CPPI_FILE_UTIL_HPP
#define CPPI_FILE_UTIL_HPP

#include <fstream>
#include <vector>

#include "token.hpp"


namespace cppi {

inline bool load_file(const char* fname, std::vector<char>& buffer) {
    buffer.clear();
    std::ifstream file(fname);
    if(!file.is_open()) {
        printf("Failed to open file %s\n", fname);
        return false;
    }
    for(std::istreambuf_iterator<char> it(file), end; it != end; ++it) {
        char c = *it;        
        switch(c) {
        case '\\': {
            ++it;
            if(it == end) {
                buffer.push_back('\n');
                break;
            }
            char next = *it;
            if(next == '\n') {
                break;
            }
            buffer.push_back('\\');
            buffer.push_back(next);
            break;
        }
        default:
            buffer.push_back(c);
        }
    }
    return true;
}
inline void dump_buffer(const std::vector<char>& buffer, const char* fname) {
    std::ofstream f(fname, std::ios::binary);
    f.write(buffer.data(), buffer.size());
    f.close();
}
inline void dump_tokens(const std::vector<token>& tokens, const char* fname) {
    std::ofstream f(fname, std::ios::binary);
    for(auto& t : tokens) {
        f.write(t.string, t.length);
    }
    f.close();
}

} // cppi


#endif
