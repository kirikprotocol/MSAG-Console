#include <memory>
#include <iostream>
#include "myfirst.pb.h"
#include "scag/util/io/HexDump.h"

using namespace scag2::util;

std::ostream& operator << ( std::ostream& o, const AbonentAddress& a )
{
    o << "<AbonentAddress ";
    if ( a.has_address() ) o << " address=\"" << a.address() << "\"";
    return o << ">";
}

std::ostream& operator << ( std::ostream& o, const Bind& b )
{
    o << "<Bind";
    if ( b.has_id() ) o << " id=" << b.id();
    if ( b.has_login() ) o << " login=\"" << b.login() << "\"";
    if ( b.has_password() ) o << " password=\"" << b.password() << "\"";
    if ( b.has_value() ) o << " value=" << b.value();
    const int ds = b.destination_size();
    if ( ds > 0 ) {
        o << " destination=[";
        for ( int i = 0; i < ds; ++i ) {
            if ( i > 0 ) o << ", ";
            o << b.destination(i);
        }
        o << "]";
    }
    return o << ">";
}

int main()
{
    Bind b;
    b.set_id(100);
    b.set_login( "hello, world" );
    b.set_password( "topsecret" );
    b.set_value( 0x123456789abcdefULL );

    b.add_destination()->set_address(".1.1.79137654079");
    b.add_destination()->set_address(".1.1.79537699490");

    std::cout << "Bind constructed:" << b << std::endl;

    const int bs = b.ByteSize();
    std::cout << "size=" << bs << std::endl;

    std::auto_ptr<google::protobuf::uint8> buf(new google::protobuf::uint8[bs]);
    b.SerializeWithCachedSizesToArray(buf.get());

    HexDump hd;
    HexDump::string_type dump;
    hd.hexdump( dump, buf.get(), bs);
    hd.strdump( dump, buf.get(), bs );
    std::cout << "dump=" << hd.c_str(dump) << std::endl;

    std::cout << "// --- reading from this buffer" << std::endl;
    Bind ob;
    google::protobuf::io::CodedInputStream cis(buf.get(),bs);
    ob.MergePartialFromCodedStream( &cis );

    std::cout << "Bind restored: " << ob << std::endl;
    return 0;
}
