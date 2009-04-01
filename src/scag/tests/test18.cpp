#include <iostream>
#include "util/int.h"

typedef int64_t  Value;

/*
#include "scag/pvss/profile/AbntAddr.hpp"
using namespace scag2::pvss;
typedef AbntAddr Key;
struct test1 {
    unsigned parent; // 4
    unsigned left;   // 4
    unsigned right;  // 4
    int8_t   color;  // 1
    Key      key;    // 24
    Value    value;  // 8
};
 */

struct test2 {
    uint64_t number;
    uint8_t  typeplan;
};

struct test3 {
    uint64_t number;
    uint8_t  typeplan;
    Value    value;
};

int main()
{
    // std::cout << "sizeof(key)=" << sizeof(Key) << std::endl;
    // std::cout << "sizeof(test1)=" << sizeof(test1) << std::endl;
    std::cout << "sizeof(test2)=" << sizeof(test2) << std::endl;
    std::cout << "sizeof(test3)=" << sizeof(test3) << std::endl;
    return 0;
}
