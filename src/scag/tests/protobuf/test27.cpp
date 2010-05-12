#include <memory>
#include <iostream>
#include "myfirst.pb.h"
#include "scag/util/io/HexDump.h"

using namespace scag2::util;

int main()
{
    Bind b;
    b.set_id(100);
    b.set_login( "hello, world" );
    b.set_password( "topsecret" );
    const int bs = b.ByteSize();
    std::cout << "size=" << bs << std::endl;

    std::auto_ptr<google::protobuf::uint8> buf(new google::protobuf::uint8[bs]);
    b.SerializeWithCachedSizesToArray(buf.get());

    HexDump hd;
    HexDump::string_type dump;
    hd.hexdump( dump, buf.get(), bs);
    hd.strdump( dump, buf.get(), bs );
    std::cout << "dump=" << hd.c_str(dump) << std::endl;

    return 0;
}
