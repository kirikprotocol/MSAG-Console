#ifndef _INFORMER_INPUTSTORAGE_H
#define _INFORMER_INPUTSTORAGE_H

#include "informer/data/InputMessageSource.h"
#include "informer/data/ActivityLog.h"
#include "informer/data/Region.h"
#include "InputJournal.h"
#include "logger/Logger.h"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

/// persistent input storage.
class InputStorage : public InputMessageSource
{
    class BlackList;
    class IReader;

    typedef std::list< InputRegionRecord >  RecordList;
    typedef smsc::core::buffers::IntHash< RecordList::iterator > RecordHash;

    class InputTransferTaskImpl : public InputTransferTask {
    public:
        InputTransferTaskImpl( TransferRequester& req,
                               unsigned count,
                               InputStorage& store ) :
        InputTransferTask(req,count), store_(store) {}
        virtual const char* taskName() { return "transtask"; }
        virtual int Execute() {
            if (!stopping()) {
                store_.doTransfer(*requester_,count_);
            }
            return 0;
        }
    private:
        InputStorage& store_;
    };

public:
    InputStorage( InputJournal&       journal );

    virtual ~InputStorage();

    virtual void init( ActivityLog& actLog );

    virtual void addNewMessages( MsgIter begin, MsgIter end );

    virtual void dropMessages( const std::vector<msgid_type>& msgids );

    virtual InputTransferTask* createInputTransferTask( TransferRequester& requester,
                                                        unsigned           count );

    // virtual MessageGlossary& getGlossary() { return glossary_; }

    virtual void setRecordAtInit(const InputRegionRecord& ro, msgid_type maxMsgId);

    virtual void postInit( std::vector<regionid_type>& regs );

    virtual size_t rollOver();

private:
    void dispatchMessages( MsgIter begin, MsgIter end,
                           smsc::core::buffers::IntHash< RegionPtr >& regs);

    /// invoked from transfertask
    void doTransfer( TransferRequester& req, size_t count );

    void getRecord(InputRegionRecord& ro, msgid_type minRlast = 0);
    void setRecord(InputRegionRecord& ro, msgid_type maxMsgId = 0);
    void doSetRecord( RecordList::iterator to, const InputRegionRecord& from );
    std::string makeFilePath(regionid_type regId,uint32_t fn) const;

    inline dlvid_type getDlvId() const { return activityLog_->getDlvId(); }
    inline DeliveryInfo& getDlvInfo() { return activityLog_->getDlvInfo(); }
    msgid_type getMinRlast();

private:
    smsc::logger::Logger*                      log_;
    smsc::core::synchronization::Mutex         wlock_;
    smsc::core::synchronization::Mutex         lock_;  // to add new regions
    RecordList                                 recordList_;
    RecordHash                                 recordHash_;
    RecordList::iterator                       rollingIter_;
    InputJournal&                              jnl_;
    uint32_t                                   lastfn_;
    ActivityLog*                               activityLog_; // not owned
    msgid_type                                 lastMsgId_;

    BlackList*                                 blackList_; // owned
};

} // informer
} // smsc

#endif
