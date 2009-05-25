#include <iostream>
#include <memory>

#include "logger/Logger.h"
#include "PvapProtocol.h"
#include "scag/util/io/HexDump.h"
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/ProfileRequest.h"

using namespace scag2::pvss;
using namespace scag2::pvss::pvap;
using namespace scag2::util;

int main()
{
    smsc::logger::Logger::Init();

    BatchCommand* batchCmd = new BatchCommand;
    ProfileRequest batch( 1, batchCmd );
    batch.getProfileKey().setAbonentKey( ".0.1.79137654079" );
    batchCmd->setTransactional( true );
    {
        std::auto_ptr< SetCommand > cmd(new SetCommand);
        cmd->setVarName("hello");
        cmd->setStringValue("хелло, ворлд");
        batchCmd->addComponent( cmd.release() );
    }

    std::cout << batch.toString() << std::endl;

    PvapProtocol protocol;
    Protocol::Buffer buf(100);
    protocol.serialize( batch, buf );

    {
        HexDump hd;
        HexDump::string_type dump;
        dump.reserve( buf.GetPos()*5 );
        hd.hexdump(dump, buf.get(), buf.GetPos());
        hd.utfdump(dump, buf.get(), buf.GetPos());
        printf( "dump: %s\n", hd.c_str(dump) );
    }

    std::auto_ptr< Packet > pack(protocol.deserialize(buf));
    std::cout << pack->toString() << std::endl;
    
    printf( "======================\n");
    printf( "setting PASSBUFFER option\n");
    protocol.setOptions( Protocol::PASSBUFFER );
    std::auto_ptr< Packet > pack2(protocol.deserialize(buf));
    std::cout << pack2->toString() << std::endl;

    {
        Protocol::Buffer buf2(100);
        protocol.setOptions(0);
        protocol.serialize( *pack2.get(), buf2 );

        std::auto_ptr< Packet > pack3(protocol.deserialize(buf2));
        std::cout << pack3->toString() << std::endl;
    }
    return 0;
}
