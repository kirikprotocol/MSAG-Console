#include <iostream>
#include <memory>

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
    ProfileRequest< BatchCommand > batch( new BatchCommand(1) );
    batch.getProfileKey().setAbonentKey( ".0.1.79137654079" );
    batch.getCommand()->setTransactional( true );
    {
        std::auto_ptr< SetCommand > cmd(new SetCommand(2));
        cmd->setVarName("hello");
        cmd->setStringValue("хелло, ворлд");
        batch.getCommand()->addComponent( cmd.release() );
    }

    std::cout << batch.toString() << std::endl;

    PvapProtocol protocol;
    Protocol::Buffer buf(100);
    protocol.serialize( batch, buf );

    {
        HexDump hd;
        unsigned sz = hd.hexdumpsize(buf.GetPos())+1;
        std::auto_ptr<char> dump(new char[sz]);
        *(hd.hexdump( dump.get(), buf.get(), buf.GetPos() )) = '\0';
        printf( "dump: %s\n", dump.get() );
    }

    std::auto_ptr< Packet > pack(protocol.deserialize(buf));
    std::cout << pack->toString() << std::endl;
    return 0;
}
