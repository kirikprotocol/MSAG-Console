#ifndef _EYELINE_SMPP_SESSION_H
#define _EYELINE_SMPP_SESSION_H

#include "informer/io/EmbedRefPtr.h"
#include "logger/Logger.h"
#include "PduListener.h"
#include "SendSubscriber.h"
#include "core/synchronization/AtomicCounter.hpp"

namespace eyeline {
namespace smpp {

/// this is inner class of smpp transport
class SessionBase;
class PduBuffer;

/// a session identified by sessionId serving a set of sockets.
/// This interface is exposed to the user.
class Session
{
    friend class eyeline::informer::EmbedRefPtr< Session >;
    friend class eyeline::informer::EmbedRefPtr< SessionBase >;

protected:
    static smsc::logger::Logger* log_;
    static void initLog();

    Session( const char* sessionId );
    virtual ~Session();

public:
    inline const char* getSessionId() const { return sessionId_.c_str(); }

    /// send pdu into network or throw an exception.
    /// NOTE: if socketId == 0 then one of the suitable output sockets will be selected.
    /// @param sub (optional) to be notified when the pdu is actually written into network.
    virtual unsigned send( unsigned        socketId,
                           PduBuffer       buffer,
                           SendSubscriber* sub = 0 ) = 0;

    /// check if session has bound socket for output
    virtual bool isBound() = 0;

private:
    void ref() 
    {
        ref_.inc();
        /*
        const unsigned refs = ref_.inc();
        if (refs==1) {
            const unsigned total = total_.inc();
            smsc_log_debug(log_,"S'%s' @%p total=%u",getSessionId(),this,total);
        }
         */
    }

    void unref() 
    {
        const unsigned refs = ref_.dec();
        if (!refs) {
            // const unsigned total = total_.dec();
            smsc_log_debug(log_,"S'%s' @%p",getSessionId(),this);
            delete this;
        }
    }

private:
    smsc::core::synchronization::AtomicCounter<unsigned>        ref_;
    static smsc::core::synchronization::AtomicCounter<unsigned> total_;
    std::string                        sessionId_;
};

typedef eyeline::informer::EmbedRefPtr< Session > SessionPtr;

}
}


#endif
