#ifndef PREPROCESS_HPP
#define PREPROCESS_HPP

#include <vector>
#include <stdio.h>
#include "util.hpp"


// Replaces all preprocessor directives with blank space
inline void preprocess(char* buf, size_t len) {
    struct line {
        char* buf;
        size_t len;
    };
    std::vector<line> lines;

    line l;
    l.buf = buf;
    l.len = 0;
    size_t cur = 0;
    size_t cur_start = 0;
    while(cur < len) {
        char c = buf[cur];
        
        if(c == '\\' && cur != len - 1 && buf[cur + 1] == '\n') {
            // Skip 2 characters
            cur += 2;
        } else if(c == '\n') {
            l.len = cur - cur_start;
            lines.push_back(l);
            l.buf = buf + cur;
            l.len = 0;
            
            cur_start = cur;
            cur++;
        } else {
            cur++;
        }
    }

    for(auto& l : lines) {
        //printf("line: %s\n", std::string(l.buf, l.len).c_str());
        size_t cur = 0;
        while(cur < l.len) {
            char c = l.buf[cur];
            if(isspace(c)) {
                ++cur;
                continue;
            } else if(ishash(c)) {
                for(int i = cur; i < l.len; ++i) {
                    l.buf[i] = ' ';
                }
                cur = l.len;
                continue;
            } else {
                cur = l.len;
                continue;
            }
        }
    }
}


#endif
