#include <iostream>
#include "scag/bill/ewallet/stream/generated-cpp/Protocol.hpp"
#include "scag/util/io/HexDump.h"

using namespace scag2::bill::ewallet;
using namespace scag2::util;

template < class Packet > void show( stream::Protocol& proto, Packet& packet ) {
    packet.setSeqNum(123456789);
    Streamer::Buffer buffer;
    stream::BufferWriter writer(buffer);
    proto.encodeMessage(packet,writer);
    HexDump hd;
    HexDump::string_type dump;
    dump.reserve(hd.hexdumpsize(buffer.GetPos()) + hd.strdumpsize(buffer.GetPos()) + 10);
    hd.hexdump(dump,buffer.get(),buffer.GetPos());
    hd.strdump(dump,buffer.get(),buffer.GetPos());
    std::cout
        << "-------------------------------------------------" << std::endl
        << "packet is: " << packet.toString() << std::endl
        << "buffer is: " << hd.c_str(dump) << std::endl
        << "-------------------------------------------------" << std::endl;
}


int main()
{
    stream::Protocol proto;
    {
        stream::Ping packet;
        show( proto, packet );
    }
    {
        stream::PingResp packet;
        packet.setStatusValue( 0x15 );
        show( proto, packet );
    }
    {
        stream::Auth packet;
        packet.setProtocolVersion(0x01);
        packet.setLogin( "bukind" );
        packet.setPassword( "password" );
        packet.setName( "Dmitry A. Bukin" );
        show( proto, packet );
    }
    {
        stream::AuthResp packet;
        packet.setStatusValue( 0x25 );
        show( proto, packet );
    }
    {
        stream::Open packet;
        packet.setAgentId( 0x12345678 );
        packet.setUserId( "bukind" );
        packet.setWalletType( "very-big-wallet" );
        packet.setDescription( "a lot of money deposit" );
        packet.setAmount( -100000 );
        packet.setTimeout( 0 );
        show( proto, packet );
    }
    {
        stream::OpenResp packet;
        packet.setStatusValue( 0 );
        packet.setTransId( 1 );
        show( proto, packet );
    }
    {
        stream::Commit packet;
        packet.setTransId( 1 );
        packet.setAmount( -100000 );
        show( proto, packet );
    }
    {
        stream::CommitResp packet;
        packet.setStatusValue( 0 );
        show( proto, packet );
    }
    {
        stream::Rollback packet;
        packet.setTransId( 15 );
        show( proto, packet );
    }
    {
        stream::RollbackResp packet;
        packet.setStatusValue( 0 );
        show( proto, packet );
    }
    return 0;
}
