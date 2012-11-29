#ifndef _EYELINE_SMPP_SERVERSESSION_H
#define _EYELINE_SMPP_SERVERSESSION_H

#include "eyeline/smpp/SmeInfo.h"
#include "eyeline/smpp/transport/SessionBase.h"

namespace eyeline {
namespace smpp {

class ServerSession : public SessionBase
{
public:
    ServerSession( const char* sessionId,
                   PduListener* listener = 0 ) :
    SessionBase(sessionId,listener) {}
    ~ServerSession();

    virtual bool removeSocket( unsigned socketId );
    void addServerSocket( Socket& socket, const SmeInfo& info );

private:

    struct SSI
    {
        SSI( const SmeInfo& info );

        static bool isWritable( BindMode bm ) {
            return bm & (BINDMODE_TRANSCEIVER|BINDMODE_RECEIVER);
        }

        // @param bindMode socket actual bind mode
        // @return true if socket is added
        bool addSocket( const SmeInfo& info, BindMode bindMode );

        // @return true if all sockets have been removed
        bool removeSocket( BindMode bindMode );

        std::string systemId;
        unsigned    nsockr;   // number of read sockets (not used for TRX)
        unsigned    nsockw;   // number of write sockets
    };

    struct SSILink
    {
        SSI*     ssi;
        BindMode bindMode; // socket actual bind mode
    };

    smsc::core::synchronization::Mutex      ssiLock_;
    smsc::core::buffers::IntHash< SSILink > ssiMap_;  // from socketId
    smsc::core::buffers::Hash< SSI* >       ssiHash_; // from systemId
};

}
}


#endif
