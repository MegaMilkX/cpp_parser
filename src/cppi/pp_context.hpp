#ifndef CPP_INSPECTOR_PREPROCESSOR_CONTEXT_HPP
#define CPP_INSPECTOR_PREPROCESSOR_CONTEXT_HPP


namespace cppi {

class pp_context {
public:
    void preprocess(const char* buffer, size_t length);
};

} // cppi


#endif
