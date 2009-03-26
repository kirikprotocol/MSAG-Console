#include <iostream>
#include "scag/pvss/profile/AbntAddr.hpp"

using namespace scag2::pvss;

typedef AbntAddr Key;
typedef int64_t  Value;

struct test1 {
    unsigned parent; // 4
    unsigned left;   // 4
    unsigned right;  // 4
    int8_t   color;  // 1
    Key      key;    // 24
    Value    value;  // 8
};

int main()
{
    std::cout << "sizeof(key)=" << sizeof(Key) << std::endl;
    std::cout << "sizeof(test1)=" << sizeof(test1) << std::endl;
    return 0;
}
