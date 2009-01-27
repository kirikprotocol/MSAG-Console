#include <iostream>
#include "PVAP.hpp"
#include "Serializer.h"

using namespace scag2::pvss::pvap;


class SamplePvapHandler : public PvapHandler
{
public:
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
    // dohandle(PC_BATCH        )
    // dohandle(PC_BATCH_RESP   )
#undef dohandle
    virtual void handle( std::auto_ptr< PC_BATCH > ) {}
    virtual void handle( std::auto_ptr< PC_BATCH_RESP > ) {}

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
        
        Serializer ss;

        PC_GET pcget;
        pcget.setSeqNum(1);
        pcget.setProfileType(0x01);
        pcget.setAbonentKey(".0.1.79137654079");
        pcget.setVarName("test_varname");

        std::cout << "pcget is:" << pcget.toString() << std::endl;

        pvap.encodeMessage(pcget,ss);
        std::cout << "pcget dump:" << ss.getDump() << std::endl;

        std::cout << "...transferring packet" << std::endl << "...processing" << std::endl;
        ss.rewind();
        pvap.decodeMessage(ss);

    } catch ( std::exception& e ) {
        std::cout << std::endl << "exception: " << e.what() << std::endl;
    }
    return 0;
}
