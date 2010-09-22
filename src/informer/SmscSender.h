#ifndef _INFORMER_SMSCSENDER_H
#define _INFORMER_SMSCSENDER_H

#include <memory>
#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "sme/SmppBase.hpp"
#include "ScoredList.h"
#include "Typedefs.h"

namespace smsc {
namespace informer {

class InfosmeCore;
class RegionSender;
class Delivery;
class Message;

/// sending messages to one smsc
class SmscSender : public smsc::core::threads::ThreadedTask, public smsc::sme::SmppPduEventListener
{
    friend class ScoredList< SmscSender >;
public:
    SmscSender( InfosmeCore&            core,
                const std::string&      smscId,
                const smsc::sme::SmeConfig& config );
    // smsc::sme::SmppSession* session );

    // InfosmeCore& getCore() { return *core_; }

    virtual ~SmscSender();

    virtual const char* taskName() { return "smscsender"; }

    /// sending one message
    /// @return number of chunks the message has been splitted or 0
    unsigned send( msgtime_type currentTime,
                   Delivery& dlv,
                   regionid_type regionId,
                   Message& msg );

    /// a method allows to wait until sender stops it work
    /// NOTE: post-requisite -- task is released!
    void updateConfig( const smsc::sme::SmeConfig& config );
    void waitUntilReleased();

private:
    virtual void handleEvent( smsc::sme::SmppHeader* pdu );
    virtual void handleError( int errorcode );

    /// sending messages
    virtual int Execute();

    virtual void onThreadPoolStartTask();
    virtual void onRelease();

    void connectLoop();
    void sendLoop();

    typedef RegionSender ScoredObjType;

    unsigned scoredObjIsReady( unsigned currentTime, ScoredObjType& regionSender );
    int processScoredObj( unsigned currentTime, ScoredObjType& regionSender );
    void scoredObjToString( std::string& s, ScoredObjType& regionSender );
    void processWaitingEvents( msgtime_type currentTime );

private:
    smsc::logger::Logger*                     log_;
    InfosmeCore*                              core_;
    std::string                               smscId_;
    std::auto_ptr<smsc::sme::SmppSession>     session_;
    smsc::core::synchronization::EventMonitor mon_;
    ScoredList< SmscSender >*                 scoredList_;
};

} // informer
} // smsc

#endif
