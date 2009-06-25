#include <iostream>
#include "scag/util/io/HexDump.h"
#include "scag/util/io/Drndm.h"

using namespace scag2::util;

int main()
{
    const size_t datalen = 100;
    const size_t passes = 10;

    std::vector< char > data;
    data.resize(datalen);
    Drndm rnd;
    rnd.setSeed(time(0));

    for ( size_t pass = 0; pass < passes; ++pass ) {
        for ( size_t i = 0; i < datalen; ++i ) {
            data[i] = char( double(rnd.getNextNumber()) / Drndm::randmax() * 256 );
        }

        HexDump hd;
        HexDump::string_type dump;
        dump.reserve(hd.hexdumpsize(datalen) + hd.strdumpsize(datalen) + 10);
        hd.hexdump(dump,&data[0],datalen);
        hd.strdump(dump,&data[0],datalen);
        std::cout << "dump: " << hd.c_str(dump) << std::endl;
    }
    return 0;
}
