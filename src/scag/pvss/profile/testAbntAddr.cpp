#include <cstdio>
#include <cstdlib>
#include "AbntAddr.hpp"
#include "scag/util/HexDump.h"

using namespace scag2::pvss;
using namespace scag2::util;
using namespace scag2::util::storage;

void compare( const AbntAddr& a )
{
    std::vector< unsigned char > buf;
    Serializer ser(buf);
    printf("serializing %s\n", a.toString().c_str());
    ser << a;

    HexDump hd;
    std::string dump;
    hd.hexdump(dump,&buf[0],buf.size());
    printf("hex: %s\n", dump.c_str());

    Deserializer deser(buf);
    AbntAddr b;
    deser >> b;
    printf("deserialized %s\n", b.toString().c_str());

    if ( a != b ) {
        fprintf(stderr,"addresses mismatch %s and %s\n", a.toString().c_str(), b.toString().c_str() );
        exit(-1);
    }
}


int main()
{
    AbntAddr a;
    AbntAddr b(11,1,1,"79137654079");
    AbntAddr c(10,1,1,"79137654079");
    compare(a);
    compare(b);
    compare(c);
    if ( a == b || b == c ) {
        fprintf(stderr,"operator == failure\n");
        exit(-1);
    }

    AbntAddr::setAllowNewPacking( false );
    compare(a);
    compare(b);
    compare(c);
    return 0;
}
