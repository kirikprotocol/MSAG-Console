#ifndef _INFORMER_SMSCSENDER_H
#define _INFORMER_SMSCSENDER_H

#include <memory>
#include "ResponseData.h"
#include "ScoredList.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "informer/data/RetryPolicy.h"
#include "informer/io/Typedefs.h"
#include "informer/opstore/RegionalStorage.h"
#include "logger/Logger.h"
#include "sme/SmppBase.hpp"

namespace smsc {
namespace sms {
class IllFormedReceiptParser;
}
}

namespace eyeline {
namespace informer {

class ReceiptProcessor;
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
    typedef smsc::core::buffers::CyclicQueue< ResponseData > DataQueue;
    typedef std::list< ReceiptData > ReceiptList;

    class SmscJournal;

    struct DRMTrans : public DlvRegMsgId {
        unsigned    nchunks;
        bool        trans;
    };

public:
    SmscSender( ReceiptProcessor&       rproc,
                const std::string&      smscId,
                const SmscConfig&       config );

    virtual ~SmscSender();

    const std::string& getSmscId() const { return smscId_; }

    const RetryPolicy& getRetryPolicy() const { return retryPolicy_; }

    void start();
    void stop();
    /// sent when something is changing
    void wakeUp();

    /// sending one message, filling nchunks.
    /// @return SMPP status
    int send( RegionalStorage& dlv, Message& msg, int& nchunks );

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
    void handleReceipt( smsc::sme::SmppHeader* pdu );
    void handleResponse( smsc::sme::SmppHeader* pdu );
    bool queueData( const ResponseData& rd );
    void processQueue( DataQueue& queue );

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
    void journalReceiptData( const ReceiptData& rd );

private:
    smsc::logger::Logger*                     log_;
    ReceiptProcessor&                         rproc_;
    RetryPolicy                               retryPolicy_;
    smsc::sms::IllFormedReceiptParser*        parser_;
    std::string                               smscId_;
    SmscConfig                                smscConfig_;

    smsc::core::synchronization::Mutex                reconfLock_;
    std::auto_ptr<smsc::sme::SmppSession>             session_;
    ScoredList< SmscSender >                          scoredList_; // not owned
    usectime_type                                     currentTime_;

    smsc::core::buffers::IntHash< DRMTrans >          seqnumHash_;
    smsc::core::buffers::CyclicQueue< ResponseTimer > respWaitQueue_;
    smsc::core::buffers::CyclicQueue< ReceiptTimer >  rcptWaitQueue_;

    // when process receipt/response.
    // monitor is used by journal also.
    smsc::core::synchronization::EventMonitor          receiptMon_;
    ReceiptList                                        receiptList_;
    ReceiptList::iterator                              rollingIter_;
    smsc::core::buffers::Hash< ReceiptList::iterator > receiptHash_;
    SmscJournal*                                       journal_;

    DataQueue*                                         rQueue_;

    // queueMon only to put into queue and notify
    smsc::core::synchronization::EventMonitor          queueMon_;
    std::auto_ptr<DataQueue>                           wQueue_;
    bool                                               awaken_;
    bool                                               isStopping_;
};

} // informer
} // smsc

#endif
