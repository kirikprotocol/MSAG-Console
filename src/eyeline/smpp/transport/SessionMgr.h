#ifndef _EYELINE_SMPP_SESSIONMGR_H
#define _EYELINE_SMPP_SESSIONMGR_H

#include "logger/Logger.h"
#include "SessionBase.h"

namespace eyeline {
namespace smpp {

class SessionMgr
{
public:
    SessionMgr();
    ~SessionMgr();

    void stop();

    bool getSession( const char* sessionId, SessionBasePtr& ptr );
    void addSession( SessionBase& ptr );
    void removeSession( const char* sessionId );

private:
    typedef smsc::core::buffers::Hash< SessionBasePtr > SessionHash;

    smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    SessionHash                        sessionHash_;
};

}
}


#endif
