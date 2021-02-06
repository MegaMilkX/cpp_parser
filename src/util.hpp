#ifndef UTIL_HPP
#define UTIL_HPP


inline bool iseof(char c) { return c == '\0'; }
inline bool isspace(char c) {
    switch(c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return true;
    default:
        return false;
    }
}
inline bool isnewline(char c) { return c == '\n'; }
inline bool isalpha(char c) {
    if(c >= 'a' && c <= 'z') {
        return true;
    } else if(c >= 'A' && c <= 'Z') {
        return true;
    } else if(c == '_') {
        return true;
    } else {
        return false;
    }
}
inline bool isnum(char c) { return (c >= '0' && c <= '9'); }
inline bool isalphanum(char c) { return isalpha(c) || isnum(c); }
inline bool ishash(char c) { return c == '#';}
inline bool iscomma(char c) { return c == ','; }
inline bool isasterisk(char c) { return c == '*'; }
inline bool istilde(char c) { return c == '~'; }
inline bool isdot(char c) { return c == '.'; }

inline bool isbackslash(char c) { return c == '\\'; }


#endif
