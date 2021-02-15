#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <memory>
#include "token.hpp"

#define REQUIRE(func) r = func(c); \
    c.advance(r); adv += r; \
    if(!r) { return 0; }
#define OPT(func) r = func(c); \
    c.advance(r); adv += r;


enum node_type {
    node_token,
    node_token_seq,
    node_brace_block,
    node_bracket_block,
    node_paren_block
};
struct node {
    node(node_type type, node* parent)
    : type(type), parent(parent) {
        tok = 0;
    }
    node_type type;
    node* parent;
    std::vector<std::unique_ptr<node>> nodes;
    token* tok;

    int token_first = 0;
    int token_count = 0;
};


struct node_cursor {
    node* sequence;
    size_t idx = 0;
    node* n = 0;

    node_cursor(node* sequence)
    : sequence(sequence) {
        if(!sequence->nodes.empty()) {
            n = sequence->nodes[idx].get();
        }
    }
    void go_up() {
        sequence = sequence->parent;
        idx = 0;
        n = sequence->nodes[idx].get();
    }
    void next() {
        ++idx;
        if(idx >= sequence->nodes.size()) {
            n = 0;
        } else {
            n = sequence->nodes[idx].get();
        }
    }
    void advance(int i = 1) {
        idx += i;
        if(idx >= sequence->nodes.size()) {
            n = 0;
        } else {
            n = sequence->nodes[idx].get();
        }
    }
    void prev(int i = 1) {
        idx -= i;
        if(idx >= sequence->nodes.size()) {
            n = 0;
        } else {
            n = sequence->nodes[idx].get();
        }
    }
    operator bool() const {
        return n != 0;
    }
    bool is_token(token_type type) const {
        if(!n) {
            if(type == tok_eof) {
                return true;
            } else {
                return false;
            }
        }
        if(n->type == node_token && n->tok->type == type) {
            return true;
        }
        return false;
    }
    bool is_any_of(const std::vector<token_type>& types) const {
        if(n->type != node_token) {
            return false;
        }
        for(auto& t : types) {
            if(n->tok->type == t) {
                return true;
            }
        }
        return false;
    }
    bool is_node(node_type type) const {
        if (!n) return false;
        return n->type == type;
    }
    void print_node(node* nd) const {
        if(nd->type == node_brace_block) {
            printf("{ ... } ");
        } else if(nd->type == node_bracket_block) {
            printf("[ ... ] ");
        } else if(nd->type == node_paren_block) {
            printf("( ... ) ");
        } else if(nd->type == node_token) {
            printf("%s ", nd->tok->get_string().c_str());
        }
    }
    void print() const {
        print_node(n);
    }
    void print_some(int count) const {
        node_cursor c = *this;
        for(size_t i = idx; i < idx + count; ++i) {
            c.print();
            c.advance();
        }
    }

    template<typename... T>
    node* try_one_of() {

    }
};


inline std::string get_identifier_adv(node_cursor& c, int& adv) {
    if(!c.is_token(tok_identifier)) {
        return std::string("");
    }
    std::string str = c.n->tok->get_string();
    adv++;
    c.advance();
    return str;
}
inline int is_tok_adv(node_cursor& c, token_type tok, int& adv) {
    if(c.is_token(tok)) {
        c.advance();
        adv++;
        return 1;
    } else {
        return 0;
    }
}
inline int is_tok(node_cursor& c, token_type tok) {
    if(c.is_token(tok)) {
        return 1;
    } else {
        return 0;
    }
}
inline int is_node_adv(node_cursor& c, node_type ntype, int& adv) {
    if(c.is_node(ntype)) {
        c.advance();
        adv++;
        return 1;
    } else {
        return 0;
    }
}

enum CLASS_KEY {
    CLASS, STRUCT, UNION
};
enum ACCESS_SPECIFIER {
    ACCESS_DEFAULT, PRIVATE, PROTECTED, PUBLIC
};
struct base_specifier {
    ACCESS_SPECIFIER    access;
    std::string         class_name;
};
struct base_clause {
    std::vector<base_specifier> specifiers;
};

inline int try_class_key(node_cursor c, CLASS_KEY& key) {
    int adv = 0;
    if(is_tok_adv(c, tok_class, adv)) {
        key = CLASS;
    } else if(is_tok_adv(c, tok_struct, adv)) {
        key = STRUCT;
    } else if(is_tok_adv(c, tok_union, adv)) {
        key = UNION;
    }
    return adv;
}

inline int try_identifier(node_cursor c, std::string& name) {
    if(c.is_token(tok_identifier)) {
        name = c.n->tok->get_string();
        return 1;
    } else {
        return 0;
    }
}
inline int try_namespace_name(node_cursor c, std::string& name) {
    return try_identifier(c, name);
}
inline int try_simple_template_id(node_cursor c, std::string& name) {
    // TODO
    int adv = 0;
    int r = try_identifier(c, name);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(!is_tok_adv(c, tok_less, adv)) {
        return 0;
    }
    while(!is_tok(c, tok_more) && !is_tok(c, tok_shift_left)) {
        if(is_tok(c, tok_eof)) {
            break;
        }
        c.advance(); adv++;
    }

    // TODO: !!!

    return 0;
}
inline int try_class_name(node_cursor c, std::string& name) {
    int r = try_identifier(c, name);
    if(r) return r;
    // TODO: simple-template-id
    return 0;
}
inline int try_enum_name(node_cursor c, std::string& name) {
    return try_identifier(c, name);
}
inline int try_typedef_name(node_cursor c, std::string& name) {
    return try_identifier(c, name);
}
inline int try_decltype_specifier(node_cursor c, std::string& name) {
    int adv = 0;
    if (!is_tok_adv(c, tok_decltype, adv)) {
        return 0;
    }
    if (!c.is_node(node_paren_block)) {
        return 0;
    }
    return 2;
}

inline int try_type_name(node_cursor c, std::string& name) {
    int r = try_simple_template_id(c, name);
    if(r) return r;
    r = try_class_name(c, name);
    if(r) return r;
    r = try_enum_name(c, name);
    if(r) return r;
    r = try_typedef_name(c, name);
    return r;
}

struct nested_name_specifier {
    std::vector<std::string> names;
};
inline int try_nested_name_specifier_tail(node_cursor c, nested_name_specifier& spec = nested_name_specifier());
inline int try_nested_name_specifier_type_name(node_cursor c, nested_name_specifier& spec = nested_name_specifier()) {
    int adv = 0;
    std::string name;
    int r = try_type_name(c, name);
    c.advance(r); adv +=r;
    if (!r) return 0;
    if(!is_tok_adv(c, tok_double_colon, adv)) {
        return 0;
    }
    spec.names.push_back(name);
    r = try_nested_name_specifier_tail(c, spec);
    c.advance(r); adv += r;
    return adv;
}
inline int try_nested_name_specifier_namespace(node_cursor c, nested_name_specifier& spec = nested_name_specifier()) {
    int adv = 0;
    std::string name;
    int r = try_namespace_name(c, name);
    c.advance(r); adv +=r;
    if (!r) return 0;
    if(!is_tok_adv(c, tok_double_colon, adv)) {
        return 0;
    }
    spec.names.push_back(name);
    r = try_nested_name_specifier_tail(c, spec);
    c.advance(r); adv += r;
    return adv;
}
inline int try_nested_name_specifier_simple_template_id(node_cursor c, nested_name_specifier& spec = nested_name_specifier()) {
    int adv = 0;
    std::string name;
    int r = try_simple_template_id(c, name);
    c.advance(r); adv +=r;
    if (!r) return 0;
    if(!is_tok_adv(c, tok_double_colon, adv)) {
        return 0;
    }
    spec.names.push_back(name);
    r = try_nested_name_specifier_tail(c, spec);
    c.advance(r); adv += r;
    return adv;
}
inline int try_nested_name_specifier_decltype(node_cursor c, nested_name_specifier& spec = nested_name_specifier()) {
    int adv = 0;
    std::string name;
    int r = try_decltype_specifier(c, name);
    c.advance(r); adv +=r;
    if (!r) return 0;
    if(!is_tok_adv(c, tok_double_colon, adv)) {
        return 0;
    }
    spec.names.push_back(name);
    r = try_nested_name_specifier_tail(c, spec);
    c.advance(r); adv += r;
    return adv;
}
inline int try_nested_name_specifier_tail(node_cursor c, nested_name_specifier& spec) {
    int r = try_nested_name_specifier_simple_template_id(c, spec);
    if(r) return r;
    r = try_nested_name_specifier_type_name(c, spec);
    if(r) return r;
    r = try_nested_name_specifier_namespace(c, spec);
    return r;
}
inline int try_nested_name_specifier(node_cursor c, nested_name_specifier& spec = nested_name_specifier()) {
    int adv = 0;
    int r = is_tok_adv(c, tok_double_colon, adv);
    bool has_prefix = r != 0;
    
    r = try_nested_name_specifier_simple_template_id(c, spec);
    c.advance(r); adv += r;
    if(r) return adv;
    r = try_nested_name_specifier_type_name(c, spec);
    c.advance(r); adv += r;
    if(r) return adv;
    r = try_nested_name_specifier_namespace(c, spec);
    c.advance(r); adv += r;
    if(r) return adv;
    if(!has_prefix) {
        r = try_nested_name_specifier_decltype(c, spec);
        c.advance(r); adv += r;
        if(r) return adv;
    }

    if(has_prefix) {
        // TODO: error
    }
    return adv;
}

inline int try_class_or_decltype_a(node_cursor c, std::string& name) {
    int adv = 0;
    nested_name_specifier nested_name;
    int r = try_nested_name_specifier(c, nested_name);
    c.advance(r); adv += r;
    r = try_class_name(c, name);
    c.advance(r); adv += r;
    if(!r) return 0;
    else return adv;
}
inline int try_class_or_decltype(node_cursor c, std::string& name) {
    int r = try_class_or_decltype_a(c, name);
    if(r) return r;
    r = try_decltype_specifier(c, name);
    return r;
}

inline int try_access_specifier(node_cursor c, ACCESS_SPECIFIER& access) {
    if(c.is_token(tok_private)) {
        access = PRIVATE;
        return 1;
    } else if(c.is_token(tok_protected)) {
        access = PROTECTED;
        return 1;
    } else if(c.is_token(tok_public)) {
        access = PUBLIC;
        return 1;
    } else {
        return 0;
    }
}

struct attribute_specifier {

};
struct attribute_specifier_seq {
    std::vector<attribute_specifier> specifiers;
};
inline int try_alignment_specifier(node_cursor c) {
    int adv = 0;
    int r = is_tok_adv(c, tok_alignas, adv);
    if(!r) return 0;
    if(!c.is_node(node_paren_block)) {
        return 0;
    }
    c.advance(); adv++;
    return adv;
}
inline int try_bracketed_attribute_specifier(node_cursor c, attribute_specifier& spec) {
    if(!c.is_node(node_bracket_block) || c.n->nodes.size() != 1) {
        return 0;
    }
    if(c.n->nodes[0]->type != node_bracket_block) {
        return 0;
    }
    node_cursor cur(c.n->nodes[0].get());
    
    return 1;
}
inline int try_attribute_specifier(node_cursor c, attribute_specifier& spec) {
    int r = try_bracketed_attribute_specifier(c, spec);
    if(r) return r;
    r = try_alignment_specifier(c);
    return r;
}
inline int try_attribute_specifier_seq(node_cursor c, attribute_specifier_seq& seq = attribute_specifier_seq()) {
    int adv = 0;
    attribute_specifier spec;
    int r = try_attribute_specifier(c, spec);
    c.advance(r); adv += r;
    if(!r) return 0;
    seq.specifiers.push_back( spec );
    r = try_attribute_specifier_seq(c, seq);
    c.advance(r); adv += r;
    return adv;
}

inline int try_base_type_specifier(node_cursor c, std::string& name) {
    return try_class_or_decltype(c, name);
}
inline int try_base_specifier_a(node_cursor c, base_specifier& spec){
    int adv = 0;
    int r = 0;

    r = try_attribute_specifier_seq(c, attribute_specifier_seq());
    c.advance(r); adv += r;

    r = try_base_type_specifier(c, spec.class_name);
    c.advance(r); adv += r;
    if(!r) { return 0; }

    spec.access = ACCESS_DEFAULT;
    return adv;
}
inline int try_base_specifier_b(node_cursor c, base_specifier& spec){
    int adv = 0;
    int r = 0;

    r = try_attribute_specifier_seq(c, attribute_specifier_seq());
    c.advance(r); adv += r;

    if(!is_tok_adv(c, tok_virtual, adv)) {
        return 0;
    }
    spec.access = ACCESS_DEFAULT;
    r = try_access_specifier(c, spec.access);
    c.advance(r); adv += r;
    r = try_base_type_specifier(c, spec.class_name);
    c.advance(r); adv += r;
    if(!r) return 0;
    return adv;
}
inline int try_base_specifier_c(node_cursor c, base_specifier& spec){
    int adv = 0;
    int r = 0;
    spec.access = ACCESS_DEFAULT;

    r = try_attribute_specifier_seq(c, attribute_specifier_seq());
    c.advance(r); adv += r;

    r = try_access_specifier(c, spec.access);
    c.advance(r); adv += r;
    is_tok_adv(c, tok_virtual, adv);
    r = try_base_type_specifier(c, spec.class_name);
    c.advance(r); adv += r;
    if(!r) return 0;
    return adv;
}
inline int try_base_specifier(node_cursor c, base_specifier& spec) {
    int r = try_base_specifier_a(c, spec);
    if(r) return r;
    r = try_base_specifier_b(c, spec);
    if(r) return r;
    r = try_base_specifier_c(c, spec);
    return r;
}
inline int try_base_specifier_list(node_cursor c, std::vector<base_specifier>& specifiers);
inline int try_base_specifier_list_a(node_cursor c, std::vector<base_specifier>& specifiers) {
    int adv = 0;
    base_specifier spec;
    int r = try_base_specifier(c, spec);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(!is_tok_adv(c, tok_comma, adv)) { return 0; }
    r = try_base_specifier_list(c, specifiers);
    c.advance(r); adv += r;
    if(!r) return 0;
    
    specifiers.push_back( spec );
    return adv;
}
inline int try_base_specifier_list_b(node_cursor c, std::vector<base_specifier>& specifiers) {
    int adv = 0;
    base_specifier spec;
    int r = try_base_specifier(c, spec);
    c.advance(r); adv += r;
    if(!r) return 0;

    specifiers.push_back( spec );
    return adv;
}
inline int try_base_specifier_list(node_cursor c, std::vector<base_specifier>& specifiers) {
    int r = try_base_specifier_list_a(c, specifiers);
    if(r) return r;
    r = try_base_specifier_list_b(c, specifiers);
    return r;
}
inline int try_base_clause(node_cursor c, base_clause& clause) {
    int adv = 0;
    if(!is_tok_adv(c, tok_colon, adv)) {
        return 0;
    }
    int r = try_base_specifier_list(c, clause.specifiers);
    c.advance(r); adv += r;
    if(!r) { return 0; }
    return adv;
}

inline int try_class_virt_specifier_adv(node_cursor& c, int& adv) {
    if(c.is_token(tok_final)) {
        c.advance();
        adv++;
        return 1;
    } else {
        return 0;
    }
}

struct declarator {
    std::string name;
};
struct init_declarator {
    declarator decl;

    void print() {
        if(!decl.name.empty()) {
            printf("%s", decl.name.c_str());
        } else {
            printf("{ no-declarator-name }");
        }
    }
};
struct init_declarator_list {
    std::vector<init_declarator> list;

    void print() {
        if(list.empty()) {
            return;
        }
        list[0].print();
        for(int i = 1; i < list.size(); ++i) {
            printf(", ");
            list[i].print();
        }
    }
};
enum CV_QUALIFIERS : uint8_t {
    CV_CONST = 0x01,
    CV_VOLATILE = 0x02
};
enum STORAGE_SPECIFIERS : uint8_t {
    STORAGE_REGISTER        = 0x01,
    STORAGE_STATIC          = 0x02,
    STORAGE_THREAD_LOCAL    = 0x04,
    STORAGE_EXTERN          = 0x08,
    STORAGE_MUTABLE         = 0x10
};
enum SIGN {
    SIGN_UNKNOWN,
    SIGN_UNSIGNED,
    SIGN_SIGNED
};
struct type_specifier {
    std::string name;
    bool is_long = false;
    SIGN sign = SIGN_UNKNOWN;

    void print() {
        printf("%s", name.c_str());
    }
};
struct decl_specifier_seq {
    type_specifier type;

    uint8_t cv;
    uint8_t storage;

    bool extern_ = false;
    bool friend_ = false;
    bool typedef_ = false;
    bool constexpr_ = false;

    void print() {
        if(storage & STORAGE_REGISTER) printf("register ");
        if(storage & STORAGE_STATIC) printf("static ");
        if(storage & STORAGE_THREAD_LOCAL) printf("thread_local ");
        if(storage & STORAGE_EXTERN) printf("extern ");
        if(storage & STORAGE_MUTABLE) printf("mutable ");

        if(cv & CV_CONST) printf("const ");
        if(cv & CV_VOLATILE) printf("volatile ");

        type.print();
    }
};
struct simple_declaration {
    attribute_specifier_seq     attributes;
    decl_specifier_seq          decl_specifiers;
    init_declarator_list        declarators;

    void print() {
        decl_specifiers.print();
        printf(" ");
        declarators.print();
    }
};
inline int try_storage_class_specifier(node_cursor c, decl_specifier_seq& seq = decl_specifier_seq()) {
    int adv = 0;
    if(is_tok_adv(c, tok_register, adv)) {
        seq.storage |= STORAGE_REGISTER;
        return 1;
    }
    if(is_tok_adv(c, tok_static, adv)) {
        seq.storage |= STORAGE_STATIC;
        return 1;
    }
    if(is_tok_adv(c, tok_thread_local, adv)) {
        seq.storage |= STORAGE_THREAD_LOCAL;
        return 1;
    }
    if(is_tok_adv(c, tok_extern, adv)) {
        seq.storage |= STORAGE_EXTERN;
        return 1;
    }
    if(is_tok_adv(c, tok_mutable, adv)) {
        seq.storage |= STORAGE_MUTABLE;
        return 1;
    }
    return 0;
}
inline int try_simple_type_specifier(node_cursor c, decl_specifier_seq& seq = decl_specifier_seq()) {
    if (!seq.type.name.empty()) { // TODO: this is a hack
        return 0;
    }
    int adv = 0;
    nested_name_specifier nested_name_spec;
    int r = try_nested_name_specifier(c, nested_name_spec);
    c.advance(r); adv += r;
    
    std::string name;
    if(r) {
        if(is_tok_adv(c, tok_template, adv)) {
            r = try_simple_template_id(c, name);
            c.advance(r); adv += r;
            if(r) {
                return adv;
            }
            return 0;
        } else {
            r = try_type_name(c, name);
            c.advance(r); adv += r;
            if(r) {
                seq.type.name = name;
                return adv;
            }
            return 0;
        }
    }

    r = try_type_name(c, name);
    if (r) {
        seq.type.name = name;
        return r;
    }
    if(is_tok(c, tok_char)) {
        seq.type.name = "char";
        return 1;
    }
    if(is_tok(c, tok_char16_t)) {
        seq.type.name = "char16_t";
        return 1;
    }
    if(is_tok(c, tok_char32_t)) {
        seq.type.name = "char32_t";
        return 1;
    }
    if(is_tok(c, tok_wchar_t)) {
        seq.type.name = "wchar_t";
        return 1;
    }
    if(is_tok(c, tok_bool)) {
        seq.type.name = "bool";
        return 1;
    }
    if(is_tok(c, tok_short)) {
        seq.type.name = "short";
        return 1;
    }
    if(is_tok(c, tok_int)) {
        seq.type.name = "int";
        return 1;
    }
    if(is_tok(c, tok_long)) {
        seq.type.is_long = true;
        return 1;
    }
    if(is_tok(c, tok_signed)) {
        seq.type.sign = SIGN_SIGNED;
        return 1;
    }
    if(is_tok(c, tok_unsigned)) {
        seq.type.sign = SIGN_UNSIGNED;
        return 1;
    }
    if(is_tok(c, tok_float)) {
        seq.type.name = "float";
        return 1;
    }
    if(is_tok(c, tok_double)) {
        seq.type.name = "double";
        return 1;
    }
    if(is_tok(c, tok_void)) {
        seq.type.name = "void";
        return 1;
    }
    if(is_tok(c, tok_auto)) {
        seq.type.name = "auto";
        return 1;
    }
    r = try_decltype_specifier(c, name);
    if (r) {
        seq.type.name = "{UNKNOWN}";
    }
    return r;
}
inline int try_cv_qualifier(node_cursor c, decl_specifier_seq& seq = decl_specifier_seq()) {
    if(is_tok(c, tok_const)) {
        seq.cv |= CV_CONST;
        return 1;
    }
    if(is_tok(c, tok_volatile)) {
        seq.cv |= CV_VOLATILE;
        return 1;
    }
    return 0;
}
inline int try_cv_qualifier_seq(node_cursor c) {
    int adv = 0;
    int r = 0;
    do {
        r = try_cv_qualifier(c);
        c.advance(r); adv += r;
    } while(r);
    return adv;
}
inline int try_ref_qualifier(node_cursor c) {
    if(is_tok(c, tok_amp)) {
        return 1;
    }
    if(is_tok(c, tok_double_amp)) {
        return 1;
    }
    return 0;
}
inline int try_trailing_type_specifier(node_cursor c, decl_specifier_seq& seq = decl_specifier_seq()) {
    int r = try_simple_type_specifier(c, seq);
    if(r) return r;
    // TODO:
    //r = try_elaborated_type_specifier(c, seq);
    //if(r) return r;
    //r = try_typename_specifier(c, seq);
    //if(r) return r;
    r = try_cv_qualifier(c, seq);
    return r;
}
inline int try_class_specifier(node_cursor c);
inline int try_enum_specifier(node_cursor c) { return 0; }
inline int try_type_specifier(node_cursor c, decl_specifier_seq& seq = decl_specifier_seq()) {
    int r = try_trailing_type_specifier(c, seq);
    if(r) return r;
    r = try_class_specifier(c); // TODO
    if(r) return r;
    r = try_enum_specifier(c); // TODO
    return r;
}
inline int try_function_specifier(node_cursor c, decl_specifier_seq& seq = decl_specifier_seq()) {
    int adv = 0;
    if(is_tok_adv(c, tok_inline, adv)) {
        
        return 1;
    }
    if(is_tok_adv(c, tok_virtual, adv)) {
        
        return 1;
    }
    if(is_tok_adv(c, tok_explicit, adv)) {
        
        return 1;
    }
    return 0;
}
inline int try_decl_specifier(node_cursor c, decl_specifier_seq& seq = decl_specifier_seq()) {
    int r = try_storage_class_specifier(c, seq);
    if(r) return r;
    r = try_type_specifier(c, seq);
    if(r) return r;
    r = try_function_specifier(c, seq);
    if(r) return r;
    r = 0;
    if(is_tok_adv(c, tok_friend, r)) {
        // TODO:
        return 1;
    }
    if(is_tok_adv(c, tok_typedef, r)) {
        // TODO:
        return 1;
    }
    if(is_tok_adv(c, tok_constexpr, r)) {
        // TODO:
        return 1;
    }
    return 0;
}
inline int try_decl_specifier_seq(node_cursor c, decl_specifier_seq& seq = decl_specifier_seq()) {
    int adv = 0;
    int r = try_decl_specifier(c, seq);
    c.advance(r); adv += r;
    if(!r) return 0;
    r = try_attribute_specifier_seq(c); // TODO?
    c.advance(r); adv += r;
    if(r) {
        return adv;
    }
    r = try_decl_specifier_seq(c, seq);
    c.advance(r); adv += r;
    return adv;
}

// === Declarators ====================
inline int try_template_id(node_cursor c) {
    return 0;
}
inline int try_unqualified_id(node_cursor c, std::string& name = std::string()) {
    int adv = 0;
    int r = try_template_id(c);
    if(r) return r;
    if(is_tok_adv(c, tok_tilde, adv)) {
        r = try_class_name(c, name);
        c.advance(r); adv += r;
        if(r) return adv;
        r = try_decltype_specifier(c, name);
        c.advance(r); adv += r;
        if(r) return adv;
        return 0;
    }
    // TODO:
    // r = try_literal_operator_id(c);
    // c.advance(r); adv += r;
    // if(r) return r;
    // r = try_conversion_function_id(c);
    // c.advance(r); adv += r;
    // if(r) return r;
    // r = try_operator_function_id(c);
    // c.advance(r); adv += r;
    // if(r) return r;
    r = try_identifier(c, name);
    return r;
}
inline int try_qualified_id(node_cursor c, std::string& name = std::string()) {
    int adv = 0;
    int r = try_nested_name_specifier(c);
    c.advance(r); adv += r;
    if(r) {
        is_tok_adv(c, tok_template, adv);
        r = try_unqualified_id(c, name);
        c.advance(r); adv += r;
        if(!r) return 0;
        return adv;
    } else {
        if(!is_tok_adv(c, tok_double_colon, adv)) {
            return 0;
        }
        r = try_template_id(c);
        c.advance(r); adv += r;
        if(r) return adv;
        // TODO:
        // r = try_literal_operator_id(c);
        // c.advance(r); adv += r;
        // if(r) return adv;
        // r = try_operator_function_id(c);
        // c.advance(r); adv += r;
        // if(r) return adv;
        r = try_identifier(c, name);
        if(r) return adv;
        return 0;
    }
}
inline int try_id_expression(node_cursor c, std::string& name = std::string()) {
    int r = try_unqualified_id(c, name);
    if(r) return r;
    r = try_qualified_id(c, name);
    return r;
}
inline int try_declarator_id(node_cursor c, declarator& decl) {
    int adv = 0;
    int r = 0;
    bool ellipsis = is_tok_adv(c, tok_elipsis, adv);
    r = try_nested_name_specifier(c);
    c.advance(r); adv += r;
    bool nested_name_spec = r != 0;
    if(!nested_name_spec) {
        r = try_id_expression(c, decl.name);
        c.advance(r); adv += r;
        if(r) return adv;
    }
    if(!ellipsis) {
        r = try_class_name(c, decl.name);
        c.advance(r); adv += r;
        if(r) return adv;
    }
    return 0;    
}
inline int try_ptr_operator(node_cursor c) {
    int adv = 0;
    if(is_tok_adv(c, tok_asterisk, adv)) {
        int r = try_attribute_specifier_seq(c);
        c.advance(r); adv += r;
        r = try_cv_qualifier_seq(c);
        c.advance(r); adv += r;
        return adv;
    }
    if(is_tok_adv(c, tok_amp, adv)) {
        int r = try_attribute_specifier_seq(c);
        c.advance(r); adv += r;
        return adv;
    }
    if(is_tok_adv(c, tok_double_amp, adv)) {
        int r = try_attribute_specifier_seq(c);
        c.advance(r); adv += r;
        return adv;
    }
    int r = try_nested_name_specifier(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(!is_tok_adv(c, tok_asterisk, adv)) {
        return 0;
    }
    r = try_attribute_specifier_seq(c);
    c.advance(r); adv += r;
    r = try_cv_qualifier_seq(c);
    c.advance(r); adv += r;
    return adv;
}
inline int try_parameters_and_qualifiers(node_cursor c);
inline int try_noptr_declarator(node_cursor c, declarator& decl) {
    int adv = 0;
    if (c.is_node(node_paren_block)) {
        return 1;
    }

    int r = try_declarator_id(c, decl);
    c.advance(r); adv += r;
    if(r) {
        adv += try_attribute_specifier_seq(c);
        return adv;
    }
    
    r = try_parameters_and_qualifiers(c);
    c.advance(r); adv += r;
    if (!r) {
        if (c.is_node(node_bracket_block)) {
            c.advance(); adv++;
            r = try_attribute_specifier_seq(c);
            c.advance(r); adv += r;
        } else {
            return 0;
        }
    }

    r = try_noptr_declarator(c, decl);
    c.advance(r); adv += r;
    return adv;
}
inline int try_noptr_abstract_pack_declarator(node_cursor c) {
    int adv = 0;
    int r = 0;
    if(is_tok(c, tok_elipsis)) {
        return 1;
    }
    r = try_parameters_and_qualifiers(c);
    c.advance(r); adv += r;
    if(!r) {
        if(!c.is_node(node_bracket_block)) {
            return 0;
        }
        c.advance(); adv++;
        r = try_attribute_specifier_seq(c);
        c.advance(r); adv += r;
    }
    r = try_noptr_abstract_pack_declarator(c);
    c.advance(r); adv += r;
    return adv;
}
inline int try_abstract_pack_declarator(node_cursor c) {
    int adv = 0;
    int r = try_noptr_abstract_pack_declarator(c);
    if(r) return r;
    r = try_ptr_operator(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    r = try_abstract_pack_declarator(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    return adv;
}
inline int try_noptr_abstract_declarator(node_cursor c) {
    int adv = 0;
    int r = 0;
    if(c.is_node(node_paren_block)) {
        return 1;
    }
    r = try_parameters_and_qualifiers(c);
    c.advance(r); adv += r;
    if(!r) {
        if(!c.is_node(node_bracket_block)) {
            return 0;
        }
        c.advance(); adv++;
        r = try_attribute_specifier_seq(c);
        c.advance(r); adv += r;
    }
    r = try_noptr_abstract_declarator(c);
    c.advance(r); adv += r;
    return adv;
}
inline int try_ptr_abstract_declarator(node_cursor c) {
    int adv = 0;
    int r = 0;
    r = try_ptr_operator(c);
    c.advance(r); adv += r;
    if(r) {
        r = try_ptr_abstract_declarator(c);
        c.advance(r); adv += r;
        return adv;
    }
    return try_noptr_abstract_declarator(c);
}
inline int try_trailing_return_type(node_cursor c) {
    // TODO:
    return 0;
}
inline int try_abstract_declarator(node_cursor c) {
    int adv = 0;
    int r = try_ptr_abstract_declarator(c);
    if(r) return r;

    r = try_noptr_abstract_declarator(c);
    c.advance(r); adv += r;
    r = try_parameters_and_qualifiers(c);
    c.advance(r); adv += r;
    if(r) {
        r = try_trailing_return_type(c);
        c.advance(r); adv += r;
        if(r) return adv;
        return 0;
    }
    if(adv == 0) {
        r = try_abstract_pack_declarator(c);
        return r;
    }
    return adv;
}
inline int try_declarator(node_cursor c, init_declarator& decl = init_declarator());
inline int try_initializer_clause(node_cursor);
inline int try_parameter_declaration(node_cursor c) {
    int adv = 0;
    int r = try_attribute_specifier_seq(c);
    c.advance(r); adv += r;
    r = try_decl_specifier_seq(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    r = try_declarator(c);
    c.advance(r); adv += r;
    if(!r) {
        r = try_abstract_declarator(c);
        c.advance(r); adv += r;
    }
    if(is_tok_adv(c, tok_assign, adv)) {
        r = try_initializer_clause(c);
        c.advance(r); adv += r;
        if(!r) return 0;
        return adv;
    }
    return adv;
}
inline int try_parameter_declaration_list(node_cursor c) {
    int adv = 0;
    int r = 0;
    r = try_parameter_declaration(c);
    while(!is_tok(c, tok_eof)) {
        if(!is_tok_adv(c, tok_comma, adv)) {
            break;
        }
        r = try_parameter_declaration(c);
        c.advance(r); adv += r;
        if(!r) {
            c.prev(); adv--; // Go back before comma
            break;
        }
    }
    return adv;
}
inline int try_parameter_declaration_clause(node_cursor c) {
    int adv = 0;
    int r = try_parameter_declaration_list(c);
    c.advance(r); adv += r;
    if(r && is_tok_adv(c, tok_comma, adv)) {
        if(is_tok_adv(c, tok_elipsis, adv)) {
            return adv;
        }
        return 0;
    }
    is_tok_adv(c, tok_elipsis, adv);
    return adv;
}
inline int try_dynamic_exception_specification(node_cursor c) {
    if(!is_tok(c, tok_throw)) {
        return 0;
    }
    c.advance();
    if(!c.is_node(node_paren_block)) {
        return 0;
    }
    return 2;
}
inline int try_noexcept_specification(node_cursor c) {
    if(!is_tok(c, tok_noexcept)) {
        return 0;
    }
    c.advance();
    if(c.is_node(node_paren_block)) {
        return 2;
    }
    return 1;
}
inline int try_exception_specification(node_cursor c) {
    int r = try_dynamic_exception_specification(c);
    if(r) return r;
    r = try_noexcept_specification(c);
    return r;
}
inline int try_parameters_and_qualifiers(node_cursor c) {
    int adv = 0;
    int r = 0;
    if(!c.is_node(node_paren_block)) {
        return 0;
    }
    node_cursor cur_inner(c.n);
    if(!try_parameter_declaration_clause(cur_inner)) {
        return 0;
    }
    c.advance(); adv++;
    r = try_attribute_specifier_seq(c);
    c.advance(r); adv += r;
    r = try_cv_qualifier_seq(c);
    c.advance(r); adv += r;
    r = try_ref_qualifier(c);
    c.advance(r); adv += r;
    r = try_exception_specification(c);
    c.advance(r); adv += r;
    return adv;
}
inline int try_ptr_declarator(node_cursor c, declarator& decl) {
    int r = try_noptr_declarator(c, decl);
    if(r) return r;
    
    int adv = 0;
    r = try_ptr_operator(c);
    c.advance(r); adv += r;
    if(r) {
        r = try_ptr_declarator(c, decl);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_declarator(node_cursor c, init_declarator& init_decl) {
    int adv = 0;
    int r = try_noptr_declarator(c, init_decl.decl);
    c.advance(r); adv += r;
    if(r) {
        r = try_parameters_and_qualifiers(c);
        c.advance(r); adv += r;
        if (r) {
            r = try_trailing_return_type(c);
            c.advance(r); adv += r;
            if (!r) return 0;
            return adv;
        }
        c.prev(adv); adv = 0;
    }

    r = try_ptr_declarator(c, init_decl.decl);
    return r;    
}
inline int try_postfix_expression(node_cursor c) {
    return 0;
}
inline int try_unary_operator(node_cursor c) {
    return 0;
}
inline int try_cast_expression(node_cursor c);
inline int try_unary_expression(node_cursor c) {
    int adv = 0;
    int r = try_postfix_expression(c);
    if(r) return r;
    if(is_tok_adv(c, tok_incr, adv)) {
        r = try_cast_expression(c);
        if(r) return adv + r;
        c.prev(adv); adv = 0;
    }
    if(is_tok_adv(c, tok_decr, adv)) {
        r = try_cast_expression(c);
        if(r) return adv + r;
        c.prev(adv); adv = 0;
    }
    r = try_unary_operator(c);
    if(r) {
        c.advance(r); adv += r;
        r = try_cast_expression(c);
        if(r) return adv + r;
        c.prev(adv); adv = 0;
    }
    if(is_tok_adv(c, tok_sizeof, adv)) {
        r = try_unary_expression(c);
        c.advance(r); adv += r;
        if(r) return adv;
        if(c.is_node(node_paren_block)) {
            adv++;
            return adv;
        }
        if(is_tok_adv(c, tok_elipsis, adv)) {
            if(c.is_node(node_paren_block)) {
                adv++;
                return adv;
            }
            c.prev();
        }
        c.prev();
    }
    if(is_tok_adv(c, tok_alignof, adv)) {
        if(c.is_node(node_paren_block)) {
            adv++;
            return adv;
        }
        c.prev();
    }/* TODO
    r = try_noexcept_expression(c);
    if(r) return r;
    r = try_new_expression(c);
    if(r) return r;
    r = try_delete_expression(c);*/
    return 0;
}
inline int try_cast_expression(node_cursor c) {
    int adv = 0;
    int r = try_unary_expression(c);
    c.advance(r); adv += r;
    if(r) return r;

    if(!c.is_node(node_paren_block)) {
        return 0;
    }
    c.advance(); adv++;
    r = try_cast_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    
    return adv;
}
inline int try_pm_expression(node_cursor c) {
    int adv = 0;
    int r = try_cast_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_dot_asterisk, adv)) {
        r = try_pm_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    } else if(is_tok_adv(c, tok_arrow_member, adv)) {
        r = try_pm_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_multiplicative_expression(node_cursor c) {
    int adv = 0;
    int r = try_pm_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_asterisk, adv)) {
        r = try_multiplicative_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    } else if(is_tok_adv(c, tok_fwd_slash, adv)) {
        r = try_multiplicative_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    } else if(is_tok_adv(c, tok_percent, adv)) {
        r = try_multiplicative_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_additive_expression(node_cursor c) {
    int adv = 0;
    int r = try_multiplicative_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_plus, adv)) {
        r = try_additive_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    } else if(is_tok_adv(c, tok_minus, adv)) {
        r = try_additive_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_shift_expression(node_cursor c) {
    int adv = 0;
    int r = try_additive_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_shift_left, adv)) {
        r = try_shift_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    } else if(is_tok_adv(c, tok_shift_right, adv)) {
        r = try_shift_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_ralational_expression(node_cursor c) {
    int adv = 0;
    int r = try_shift_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_less, adv)) {
        r = try_ralational_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    } else if(is_tok_adv(c, tok_more, adv)) {
        r = try_ralational_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    } else if(is_tok_adv(c, tok_less_assign, adv)) {
        r = try_ralational_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    } else if(is_tok_adv(c, tok_more_assign, adv)) {
        r = try_ralational_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_equality_expression(node_cursor c) {
    int adv = 0;
    int r = try_ralational_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_equals, adv)) {
        r = try_equality_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    } else if(is_tok_adv(c, tok_not_eq, adv)) {
        r = try_equality_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_and_expression(node_cursor c) {
    int adv = 0;
    int r = try_equality_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_amp, adv)) {
        r = try_and_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_exclusive_or_expression(node_cursor c) {
    int adv = 0;
    int r = try_and_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_hat, adv)) {
        r = try_exclusive_or_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_inclusive_or_expression(node_cursor c) {
    int adv = 0;
    int r = try_exclusive_or_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_pipe, adv)) {
        r = try_inclusive_or_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_logical_and_expression(node_cursor c) {
    int adv = 0;
    int r = try_inclusive_or_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_double_amp, adv)) {
        r = try_logical_and_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_logical_or_expression(node_cursor c) {
    int adv = 0;
    int r = try_logical_and_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_double_pipe, adv)) {
        r = try_logical_or_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
    }
    return adv;
}
inline int try_expression(node_cursor c) {
    // TODO?
    return 0;
}
inline int try_assignment_expression(node_cursor c);
inline int try_conditional_expression(node_cursor c) {
    int adv = 0;
    int r = try_logical_or_expression(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    if(is_tok_adv(c, tok_question, adv)) {
        r = try_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
        if(!is_tok_adv(c, tok_colon, adv)) {
            return 0;
        }
        r = try_assignment_expression(c);
        c.advance(r); adv += r;
        if(!r) return 0;
        return adv;
    }
    return adv;
}
inline int try_assignment_operator(node_cursor& c) {
    if(is_tok(c, tok_assign)) return 1;
    if(is_tok(c, tok_asterisk_assign)) return 1;
    if(is_tok(c, tok_fwd_slash_assign)) return 1;
    if(is_tok(c, tok_percent_assign)) return 1;
    if(is_tok(c, tok_plus_assign)) return 1;
    if(is_tok(c, tok_minus_assign)) return 1;
    if(is_tok(c, tok_shift_right_assign)) return 1;
    if(is_tok(c, tok_shift_left_assign)) return 1;
    if(is_tok(c, tok_amp_assign)) return 1;
    if(is_tok(c, tok_hat_assign)) return 1;
    if(is_tok(c, tok_pipe_assign)) return 1;
    return 0;
}
inline int try_throw_expression(node_cursor c) {
    return 0;
}
inline int try_assignment_expression(node_cursor c) {
    int adv = 0;
    int r = try_conditional_expression(c);
    if(r) return r;

    r = try_logical_or_expression(c);
    c.advance(r); adv += r;
    if(r) {
        r = try_assignment_operator(c);
        c.advance(r); adv += r;
        if(!r) return 0;
        r = try_initializer_clause(c);
        c.advance(r); adv += r;
        if(!r) return 0;
        return adv;
    }

    r = try_throw_expression(c);
    c.advance(r); adv += r;
    return r;
}
inline int try_braced_init_list(node_cursor c) {
    if(c.is_node(node_brace_block)) {
        return 1;
    }
    return 0;
}
inline int try_initializer_clause(node_cursor c) {
    int r = try_assignment_expression(c);
    if(r) return r;
    r = try_braced_init_list(c);
    return r;
}
inline int try_brace_or_equal_initializer(node_cursor c) {
    int adv = 0;
    if(is_tok_adv(c, tok_assign, adv)) {
        /* No need to parse the expression
        int r = try_initializer_clause(c);
        c.advance(r); adv += r;
        if(!r) return 0;
        return adv;*/
        // Hack to skip expression parsing
        while(!is_tok(c, tok_semicolon) && !is_tok(c, tok_comma) && !is_tok(c, tok_eof)) {
            c.advance(); adv++;
        }
        return adv;
    }

    int r = try_braced_init_list(c);
    c.advance(r); adv += r;
    return adv;
}
inline int try_initializer(node_cursor c) {
    int r = try_brace_or_equal_initializer(c);
    if(r) return r;
    if(c.is_node(node_paren_block)) {
        // TODO: expression-list
        return 1;
    }
    return 0;
}
inline int try_init_declarator(node_cursor c, init_declarator& init_decl) {
    int adv = 0;
    int r = try_declarator(c, init_decl);
    c.advance(r); adv += r;
    if(!r) return 0;
    r = try_initializer(c);
    c.advance(r); adv += r;
    return adv;
}
inline int try_init_declarator_list(node_cursor c, init_declarator_list& list) {
    int adv = 0;
    init_declarator init_decl;
    int r = try_init_declarator(c, init_decl);
    c.advance(r); adv += r;
    if(!r) return 0;
    list.list.push_back(init_decl);
    if(!is_tok_adv(c, tok_comma, adv)) {
        return adv;
    }
    r = try_init_declarator_list(c, list);
    c.advance(r); adv += r;
    if(!r) return 0;
    return adv;
}

// === simple-declaration =============

inline int try_simple_declaration(node_cursor c, simple_declaration& decl = simple_declaration()) {
    int start = c.idx;
    int adv = 0;
    int r = try_attribute_specifier_seq(c, decl.attributes);
    c.advance(r); adv += r;
    bool declarator_required = false;
    if(r) { declarator_required = true; }

    r = try_decl_specifier_seq(c, decl.decl_specifiers);
    c.advance(r); adv += r;

    r = try_init_declarator_list(c, decl.declarators);
    c.advance(r); adv += r;
    if(declarator_required && !r) {
        return 0; 
    }

    if(!is_tok_adv(c, tok_semicolon, adv)) {
        return 0;
    }

    if(adv) {
        printf("simple-declaration: ");
        decl.print();
        printf("\n");
        printf("\tunparsed: ");
        for(int i = start; i < start + adv; ++i) {
            node* n = c.sequence->nodes[i].get();
            if(n->type == node_token) {
                printf("%s ", n->tok->get_string().c_str());
            } else if(n->type == node_brace_block) {
                printf("{ ... } ");
            } else if(n->type == node_bracket_block) {
                printf("[ ... ] ");
            } else if(n->type == node_paren_block) {
                printf("( ... ) ");
            }
        }
        printf("\n");
    }
    return adv;
}

// === function-definition ============

struct function_definition {
    attribute_specifier_seq attribs;
    declarator declarator_;
};
inline int try_virt_specifier(node_cursor c) {
    if(is_tok(c, tok_override)) {
        return 1;
    }
    if(is_tok(c, tok_final)) {
        return 1;
    }
    return 0;
}
inline int try_virt_specifier_seq(node_cursor c) {
    int adv = 0;
    int r = try_virt_specifier(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    r = try_virt_specifier_seq(c);
    c.advance(r); adv += r;
    return adv;
}
inline int try_function_body(node_cursor c) {
    if(!c.is_node(node_brace_block)) {
        return 0;
    }
    return 1;
}
inline int try_function_definition(node_cursor c, function_definition& def = function_definition()) {
    int adv = 0;
    int r = try_attribute_specifier_seq(c, def.attribs);
    c.advance(r); adv += r;
    r = try_decl_specifier_seq(c);
    c.advance(r); adv += r;
    r = try_declarator(c);
    c.advance(r); adv += r;
    if(!r) return 0;
    r = try_virt_specifier_seq(c);
    c.advance(r); adv += r;
    r = try_function_body(c);
    c.advance(r); adv += r;
    if(!r) return 0;

    printf("function-definition: ");
    printf("\n");

    return adv;
}

// === Class Definition ===============

struct class_definition {
    nested_name_specifier nested_name_spec;
    std::string name;
    base_clause base;
    attribute_specifier_seq attribs;
    bool is_final;
};

inline int try_class_head_name(node_cursor c, class_definition& def = class_definition()) {
    int adv = 0;
    int r = try_nested_name_specifier(c, def.nested_name_spec);
    c.advance(r); adv += r;
    r = try_class_name(c, def.name);
    c.advance(r); adv += r;
    if(!r) return 0;
    return adv;
}
inline int try_class_head_a(node_cursor c, class_definition& def = class_definition()) {
    int adv = 0;
    CLASS_KEY key;
    int r = try_class_key(c, key);
    c.advance(r); adv += r;
    if(!r) { return 0; }

    r = try_attribute_specifier_seq(c, def.attribs);
    c.advance(r); adv += r;

    r = try_class_head_name(c, def);
    c.advance(r); adv += r;
    if(!r) return 0;
    
    def.is_final = try_class_virt_specifier_adv(c, adv) > 0;
    
    r = try_base_clause(c, def.base);
    c.advance(r); adv += r;
    return adv;
}
inline int try_class_head_b(node_cursor c, class_definition& def = class_definition()) {
    int adv = 0;
    CLASS_KEY key;
    int r = try_class_key(c, key);
    c.advance(r); adv += r;
    if(!r) { return 0; }

    r = try_attribute_specifier_seq(c, def.attribs);
    c.advance(r); adv += r;

    def.is_final = false;
    def.name = "";
    
    r = try_base_clause(c, def.base);
    c.advance(r); adv += r;
    return adv;
}
inline int try_class_head(node_cursor c, class_definition& def = class_definition()) {
    int r = try_class_head_a(c, def);
    if(r) return r;
    r = try_class_head_b(c, def);
    return r;
}
inline int try_class_specifier(node_cursor c) {
    class_definition def;
    int adv = 0;
    int r = try_class_head(c, def);
    c.advance(r); adv += r;
    if(!r) { return 0; }

    if(!is_node_adv(c, node_brace_block, adv)) { return 0; }

    printf("class-specifier: ");
    if(!def.nested_name_spec.names.empty()) {
        for(int i = 0; i < def.nested_name_spec.names.size(); ++i) {
            printf("%s::", def.nested_name_spec.names[i].c_str());
        }
    }
    printf("%s", def.name.c_str());
    if(!def.base.specifiers.empty()) {
        printf(", base classes(%i): ", def.base.specifiers.size());
        for(int i = 0; i < def.base.specifiers.size(); ++i) {
            printf("(%s) ", def.base.specifiers[i].class_name.c_str());
        }
    }
    printf("\n");
    return adv;
}


#endif
