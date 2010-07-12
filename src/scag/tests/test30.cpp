// a test of map invariants

#include <map>
#include <iostream>

typedef std::map<int,int> IntMap;
const void* showiter( IntMap::const_iterator i ) {
    return (i.operator->());
}

int main()
{

    IntMap themap;
    IntMap::iterator i = themap.begin();
    IntMap::iterator j = themap.end();
    std::cout << "i == map.begin() = " << showiter(i) << std::endl;
    std::cout << "j == map.end() = " << showiter(j) << std::endl;
    std::cout << "i == j ? " << (i == j) << std::endl;

    themap.insert( IntMap::value_type(10,10) );
    std::cout << "i = " << showiter(i) << std::endl;
    std::cout << "j = " << showiter(j) << std::endl;
    std::cout << "map.begin() = " << showiter(themap.begin()) << std::endl;
    std::cout << "map.end() = " << showiter(themap.end()) << std::endl;
    std::cout << "i == map.begin() ? " << ( i == themap.begin() ) << std::endl;
    std::cout << "j == map.end() ? " << ( j == themap.end() ) << std::endl;
    return 0;
}
