#include <iostream>
#include <cstdlib>

#ident "hello, world"

int main()
{
#define showenv(x) std::cout << x << "=" << ( std::getenv(x) ? std::getenv(x) : "<not set>" ) << std::endl
    showenv("LC_ALL");
    showenv("LC_CTYPE");
    showenv("LANG");
    return 0;
}
