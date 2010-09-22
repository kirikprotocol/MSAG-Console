#include <iostream>
#include <cstdlib>

int main()
{
#define showenv(x) std::cout << x << "=" << ( getenv(x) ? getenv(x) : "<not set>" ) << std::endl
    showenv("LC_ALL");
    showenv("LC_CTYPE");
    showenv("LANG");
    return 0;
}
