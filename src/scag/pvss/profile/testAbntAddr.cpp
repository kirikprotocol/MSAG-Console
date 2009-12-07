#include <cstdio>
#include <cstdlib>
#include "AbntAddr.hpp"
#include "scag/util/io/HexDump.h"

using namespace scag2::pvss;
using namespace scag2::util;
using namespace scag2::util::io;

void compare( const AbntAddr& a )
{
    std::vector< unsigned char > buf;
    Serializer ser(buf);
    printf("serializing %s\n", a.toString().c_str());
    ser << a;

    HexDump hd;
    HexDump::string_type dump;
    hd.hexdump(dump,&buf[0],buf.size());
    printf("hex: %s\n", hd.c_str(dump));

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
    AbntAddr::setAllowNewPacking( true );
    printf("using new packing\n");

    AbntAddr a;
    // AbntAddr b(11,1,1,"79137654079");
    // AbntAddr b(11,1,1,"21000001575");
    // AbntAddr c(11,1,1,"79101799991");
    AbntAddr b; b.fromString(".1.1.21000001575 ");
    AbntAddr c; c.fromString("4790002120422769 ");
    compare(a);
    compare(b);
    compare(c);
    if ( a == b || b == c ) {
        fprintf(stderr,"operator == failure\n");
        exit(-1);
    }

    AbntAddr::setAllowNewPacking( false );
    printf("using old packing\n");

    compare(a);
    compare(b);
    compare(c);
    return 0;
}
