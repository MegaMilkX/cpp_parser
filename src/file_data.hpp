#ifndef FILE_DATA_HPP
#define FILE_DATA_HPP

#include <vector>
#include <map>
#include <string>

struct Type {
    std::string     name;
};

struct Variable {
    Type            type;
    std::string     name;
};

struct Enum {
    std::string                 name;
    std::vector<std::string>    elems;
};

struct Function {
    std::string             name;
    Type                    return_type;
    std::vector<Type>       parameters;
};

struct Class {
    std::string             name;
    std::vector<Variable>   variables;
    std::vector<Function>   functions;
    std::vector<Class>      classes;
    std::vector<Enum>       enums;
};

struct Namespace {
    std::string             name;
    std::vector<Variable>   variables;
    std::vector<Function>   functions;
    std::vector<Class>      classes;
    std::vector<Enum>       enums;
    std::vector<Namespace>  namespaces;

    std::map<std::string, size_t> classes_by_name;
    std::map<std::string, size_t> enums_by_name;
    std::map<std::string, size_t> variables_by_name;
    std::map<std::string, size_t> namespaces_by_name;
    std::map<std::string, size_t> functions_by_name;

    void addVariable(const Variable& var) {
        variables_by_name[var.name] = variables.size();
        variables.push_back(var);
    }
    void addFunction(const Function& func) {
        functions_by_name[func.name] = functions.size();
        functions.push_back(func);
    }
    void addClass(const Class& class_) {
        classes_by_name[class_.name] = classes.size();
        classes.push_back(class_);
    }
    void addEnum(const Enum& enum_) {
        enums_by_name[enum_.name] = enums.size();
        enums.push_back(enum_);
    }
    void addNamespace(const Namespace& namespace_) {
        namespaces_by_name[namespace_.name] = namespaces.size();
        namespaces.push_back(namespace_);
    }
};

struct FileData {
    std::vector<Variable>   variables;
    std::vector<Class>      classes;
    std::vector<Namespace>  namespaces;
};


#endif
