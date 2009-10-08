//
// A test of map for stdcxx
//
#include <map>
#include <list>
#include <string>
#include <iostream>
#include <time.h>

struct Tmp
{
    struct MappingValue;

    typedef std::list< MappingValue >                    ValueList;
    typedef std::map< int, ValueList::iterator >         AbonentMap;
    typedef std::multimap< time_t, ValueList::iterator > TimeoutMap;

    struct MappingValue
    {
        MappingValue( const std::string& d) : data(d) {}

        std::string data;

        AbonentMap::iterator aiter;
        TimeoutMap::iterator titer;
    };

    void createMapping( int addr, const std::string& data );

    AbonentMap abonentMap;
    TimeoutMap timeoutMap;
    ValueList  valueList;
};


std::ostream& operator << ( std::ostream& o, const Tmp::MappingValue& v ) {
    return o << v.data;
}


void Tmp::createMapping( int addr, const std::string& data )
{
    ValueList::iterator i = valueList.insert(valueList.begin(),MappingValue(data));
    i->aiter = abonentMap.insert(std::make_pair(addr,i)).first;
    i->titer = timeoutMap.insert(std::make_pair(time(0),i));
    std::cout << "key:" << (i->aiter->first) << ", val:" << i->data << std::endl;
}


int main()
{
    Tmp tmp;
    tmp.createMapping( 10, "hello" );
    tmp.createMapping( 20, "world" );
    return 0;
}
