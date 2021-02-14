
#include <stdint.h>
#include <string>

#include "test_include.hpp"

#if defined TEST_INCLUDED
static const int TEST_INCL;
#endif

#define JELLO

#if !defined (JELLO)
static const int JELLO_0;
#endif

#if defined (JELLO)
static const int JELLO_1;
#endif

#if defined JELLO
static const int JELLO_2;
#endif
#if !defined JELLO
static const int JELLO_3;
#endif

#if -(10 + 20)
#endif

#if 10 + 3
#endif

#if 10 + 5 * 10
#endif
#if (10 + 5) * 10
#endif
#if 10 ? (66 + 600) : (666 / 6)
#elif true != 0
#endif

#if 0 / 1 * 10 << 19 % (100 + 10) < 100 + 500 * 32 + (10 & 8) && true
static const int ASS;
#endif

#ifdef AAA
#ifdef BBB
static const int POO1;
#else
static const int BOO1;
#endif
#else
static const int ZOO1;
#endif

#define AAA
#ifdef AAA
#ifdef BBB
static const int POO2;
#else
static const int BOO2;
#endif
#else
static const int ZOO2;
#endif
#undef AAA

#define BBB
#ifdef AAA
#ifdef BBB
static const int POO3;
#else
static const int BOO3;
#endif
#else
static const int ZOO3;
#endif
#undef BBB

#define AAA
#define BBB
#ifdef AAA
#ifdef BBB
static const int POO4;
#else
static const int BOO4;
#endif
#else
static const int ZOO4;
#endif
#undef AAA
#undef BBB

#define EMPTY(X)
#define VARIADIC(A,B, ...) static const char* A = (#__VA_ARGS__)
VARIADIC(a, b, asda, "asdsa"  ,asd   ssd   , asd          );

#define MAC(A, B) 
MAC(a, b, c , d e, e)

EMPTY(A);
//EMPTY

#define MY_MACRO static_assert(false)
#define MERGE2(A, B) A ## _ ## B
MERGE2(MY, MACRO);

#define DEFINE_STRING(VAR, TEXT) static const char* VAR = #TEXT
DEFINE_STRING(my_string, test
 = "Hello \"World!\" <> @#@ 2" foo);

#define MISTAKE          lol ##;                      

#define MYMACRO int foo(void) { \
    return 0;\
}

#define MYMACRO2( NAME , POOP , ...) class NAME {}

MYMACRO2 (
    ClassTest, 
    (GOOP), 
    ((WHATEVS, POOP))
);
MISTAKE
#define FOO
#define BAR

#pragma hello
#pragma once

template<int VALUE>
class value_object {};

template<typename T>
class value_array{};

template<typename T = value_array< value_object<(5 < 4)> >>
class value_test{};

static const value_array<value_object<(1 >> 4)>> my_array;
static const value_test<int> my_test;

static const int hello = 10;

[[]]
bool val;

[[Attrib]];

decltype(2) boob;

struct MyStruct {};

class A {
    template<typename T>
    void foo();
};
template<typename T>
void A:: template foo<T>();

enum EnumBase {
    POO
};

enum [[A]] Enum : unsigned int {
    FIRST,
    SECOND,
    THIRD
};

void kek(unsigned int a) {

}

void foo() {
    
}

[[A]]
const volatile int [[A]]* const volatile [[A]] poop2 [[A]];

class bar {};

const int typedef lol;

extern int(*gloop)(int);

inline int poop(float value) {
    return 0;
}
inline int poop_3(float booba);

namespace Hidden {
    class BaseClass {
        int foo();
    };
    int BaseClass::foo() {
        return 0;
    }
}

class BaseA {};
class BaseB {};
class BaseC {};
class BaseD {};
class BaseE {};

class [[]] MyClassZ final : BaseA, public BaseB, [[Poop]] virtual BaseC, virtual private BaseD, protected virtual BaseE {

};

class [[]] : BaseA {};

namespace Test {
    namespace Z0 {
        class MyClass;
    }
}

static const BaseA base_a;

class
[[ Engine::Actor(health = 10 { mana=9, cuteness=3 }), Category(Stuff), Editor, Reflect ]] 
::Test::Z0::MyClass : public ::Hidden::BaseClass {
public:
    void foo(int a, float b) {

    }
};