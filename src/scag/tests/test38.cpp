#include <stdio.h>
#include <stdexcept>
#include <string>

// #include <bits/dballocator.h>

/*
struct A
{
    A() : dummy(false) {
        data = "hello";
    }
    const char* data;
    bool dummy;
};
 */

    
using namespace std;

// typedef std::basic_string<char,std::char_traits<char>,std::dballocator<char> > dbstring;
// typedef std::basic_string<char,std::char_traits<char>,std::allocator<char> > dbstring;

int main()
{
    string hello;
    hello.reserve(100);
    hello = "hello";
    printf("%s\n",hello.c_str());
    throw std::logic_error("hello");

    /*
    std::vector< A > avec;
    avec.reserve(13);
    avec.push_back( A() );
    printf("%s\n",avec[0].data);
     */

    /*
    typedef std::basic_string<char,std::char_traits<char>,std::dballocator<char>> dbstring;
    dbstring hi;
    hi.reserve(100);
    hi = "hello";
    printf("%s\n",hi.c_str());
     */

    return 0;
}
