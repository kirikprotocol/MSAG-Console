#include <iostream>
#include <memory>

#include "logger/Logger.h"
#include "PvapProtocol.h"
#include "scag/util/HexDump.h"
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
        std::string dump;
        dump.reserve( buf.GetPos()*5 );
        hd.hexdump(dump, buf.get(), buf.GetPos());
        hd.utfdump(dump, buf.get(), buf.GetPos());
        printf( "dump: %s\n", dump.c_str() );
    }

    std::auto_ptr< Packet > pack(protocol.deserialize(buf));
    std::cout << pack->toString() << std::endl;
    return 0;
}
