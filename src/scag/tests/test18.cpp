#include <iostream>
#include "scag/pvss/profile/AbntAddr.hpp"

using namespace scag2::pvss;

typedef AbntAddr Key;
typedef int64_t  Value;

struct test1 {
    unsigned parent;
    unsigned left;
    unsigned right;
    int8_t   color;
    Key      key;
    Value    value;
};

int main()
{
    std::cout << "sizeof(test1)=" << sizeof(test1) << std::endl;
    return 0;
}
