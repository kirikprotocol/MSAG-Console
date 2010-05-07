#include <iostream>
#include "util/int.h"

struct Empty
{
};

struct EmptyHolder
{
    int64_t val;
    Empty   empty;
};

struct A
{
    int x;
};

struct AHolder
{
    A a;
};


int main()
{
#define SHOWSIZE(x) std::cout << "sizeof(" #x ") = " << sizeof(x) << std::endl;

    SHOWSIZE(int8_t);
    SHOWSIZE(short);
    SHOWSIZE(int);
    SHOWSIZE(long);
    SHOWSIZE(long long);
    SHOWSIZE(struct Empty);
    SHOWSIZE(struct EmptyHolder);
    SHOWSIZE(struct A);
    SHOWSIZE(struct AHolder);
    return 0;
}
