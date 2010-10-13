#ifndef _INFORMER_SMSCSENDER_H
#define _INFORMER_SMSCSENDER_H

#include <memory>
#include "ScoredList.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "informer/io/Typedefs.h"
#include "logger/Logger.h"
#include "sme/SmppBase.hpp"
#include "informer/opstore/RegionalStorage.h"

namespace eyeline {
namespace informer {

class InfosmeCore;
class RegionSender;
class Message;

struct SmscConfig
{
    smsc::sme::SmeConfig smeConfig;
    int ussdPushOp;
    int ussdPushVlrOp;
};


/// sending messages to one smsc
class SmscSender : public smsc::core::threads::Thread, public smsc::sme::SmppPduEventListener
{
    friend class ScoredList< SmscSender >;
public:
    SmscSender( InfosmeCore&            core,
                const std::string&      smscId,
                const SmscConfig&       config );

    virtual ~SmscSender();

    const std::string& getSmscId() const { return smscId_; }

    void start();
    void stop();

    /// sending one message
    /// @return number of chunks the message has been splitted or 0
    unsigned send( RegionalStorage&    dlv,
                   Message&            msg );

    /// a method allows to wait until sender stops it work
    /// NOTE: post-requisite -- task is released!
    void updateConfig( const SmscConfig& config );
    // void waitUntilReleased();

    /// this two methods are invoked from locked state.
    void detachRegionSender( RegionSender& rs );
    void attachRegionSender( RegionSender& rs );

private:
    virtual void handleEvent( smsc::sme::SmppHeader* pdu );
    virtual void handleError( int errorcode );

    /// sending messages
    virtual int Execute();

    // virtual void onThreadPoolStartTask();
    // virtual void onRelease();

    void connectLoop();
    void sendLoop();

    typedef RegionSender ScoredObjType;

    unsigned scoredObjIsReady( unsigned unused, ScoredObjType& regionSender );
    int processScoredObj( unsigned unused, ScoredObjType& regionSender );
    void scoredObjToString( std::string& s, ScoredObjType& regionSender );
    void processWaitingEvents();

private:
    smsc::logger::Logger*                     log_;
    InfosmeCore*                              core_;
    std::string                               smscId_;
    std::auto_ptr<smsc::sme::SmppSession>     session_;
    smsc::core::synchronization::EventMonitor mon_;
    ScoredList< SmscSender >                  scoredList_; // not owned
    usectime_type                             currentTime_;
    bool                                      isStopping_;
    int                                       ussdPushOp_;
    int                                       ussdPushVlrOp_;
};

} // informer
} // smsc

#endif
