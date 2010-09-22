/// a test of multimap sequential insertion

#include <iostream>
#include <map>

int main()
{
    typedef std::multimap<int,int> Themap;
    Themap themap;
    for ( int i = 0; i < 100; ++i ) {
        themap.insert(std::make_pair(0,i) );
    }
    for ( Themap::const_iterator i = themap.begin();
          i != themap.end(); ++i ) {
        std::cout << i->second << std::endl;
    }
    return 0;
}
