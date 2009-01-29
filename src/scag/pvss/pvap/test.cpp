#include <iostream>
#include "PVAP.hpp"
#include "Serializer.h"

using namespace scag2::pvss::pvap;


class SamplePvapHandler : public PVAP::Handler
{
public:
    virtual bool hasSeqNum( uint32_t seqnum ) const { return true; }

#define dohandle(x) virtual void handle(std::auto_ptr<x> msg) { this->dump(#x,msg.get()); }
    dohandle(PC_DEL          );
    dohandle(PC_DEL_RESP     );
    dohandle(PC_SET          );
    dohandle(PC_SET_RESP     );
    dohandle(PC_GET          );
    dohandle(PC_GET_RESP     );
    dohandle(PC_INC          );
    dohandle(PC_INC_RESP     );
    dohandle(PC_INC_MOD      );
    dohandle(PC_INC_MOD_RESP );
    dohandle(PC_PING         );
    dohandle(PC_PING_RESP    );
    dohandle(PC_AUTH         );
    dohandle(PC_AUTH_RESP    );
    dohandle(PC_BATCH        );
    dohandle(PC_BATCH_RESP   );
#undef dohandle

private:
    template <class CMD> void dump( const char* msgType, CMD* msg )
    {
        std::cout << "got: " << msg->toString() << std::endl;
    }
};

int main()
{
    try {
        SamplePvapHandler handler;

        PVAP pvap;
        pvap.assignHandler( &handler );
        
        Serializer writer;

        {
            PC_BATCH pcbatch;
            pcbatch.setSeqNum(1);
            pcbatch.setProfileType(0x01);
            pcbatch.setAbonentKey(".0.1.79137654079");
            pcbatch.setBatchMode( true );
            BatchCmdArray cmds;
            {
                std::auto_ptr<BC_GET> bcget(new BC_GET);
                bcget->setSeqNum(2);
                // pcget->setProfileType(0x01);
                // pcget->setAbonentKey(".0.1.79137654079");
                bcget->setVarName("test_varname");
                cmds.push( bcget.release() );

                std::auto_ptr<BC_SET> bcset(new BC_SET);
                bcset->setSeqNum(3);
                bcset->setVarName("test_setvar");
                bcset->setValueType(0x2);
                bcset->setTimePolicy(0x1);
                bcset->setFinalDate(100);
                bcset->setLifeTime(200);
                bcset->setStringValue( "хелло, ворлд" );
                cmds.push( bcset.release() );
            }
            pcbatch.setBatchContent( cmds );
            
            std::cout << "pcbatch is:" << pcbatch.toString() << std::endl;

            pvap.encodeMessage(pcbatch,writer);
            std::cout << "writer dump:" << writer.getDump() << std::endl;
        }
        // writer.rewind();

        std::cout << "...transferring packet" << std::endl << "...processing" << std::endl;

        {
            Serializer reader;
            reader.setData( writer.getData(), writer.getPos(), false );
            pvap.decodeMessage(reader);
        }

    } catch ( std::exception& e ) {
        std::cout << std::endl << "exception: " << e.what() << std::endl;
    }
    return 0;
}
