#ifndef LOAD_FILE_HPP
#define LOAD_FILE_HPP

#include <vector>
#include <fstream>
#include <stdio.h>

// Load entire text file into a char buffer
inline bool load_file(const char* fname, std::vector<char>& buffer) {
    std::ifstream file(fname);
    if (!file.is_open()) {
        printf("Failed to open file\n");
        return false;
    }
    file.seekg(0, std::ios::end); // go to the end
    size_t length = file.tellg(); // report location (this is the length)
    file.seekg(0, std::ios::beg);
    
    buffer.resize(length);
    file.read((char*)buffer.data(), length);
    buffer.push_back('\0');
    buffer.push_back('\0');
    return true;
}

inline bool load_file2(const char* fname, std::vector<char>& buffer) {
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

#endif
