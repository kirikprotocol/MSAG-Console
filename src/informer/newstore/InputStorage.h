#ifndef _INFORMER_INPUTSTORAGE_H
#define _INFORMER_INPUTSTORAGE_H

#include "informer/data/InputMessageSource.h"
#include "informer/data/MessageGlossary.h"
#include "informer/data/ActivityLog.h"
#include "InputJournal.h"
#include "logger/Logger.h"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class InfosmeCore;

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
    InputStorage( DeliveryActivator&  core,
                  InputJournal&       journal );

    virtual ~InputStorage();

    virtual void init( ActivityLog& actLog );

    virtual DeliveryActivator& getDlvActivator() { return core_; }

    virtual void addNewMessages( MsgIter begin, MsgIter end );

    virtual void dropMessages( const std::vector<msgid_type>& msgids );

    virtual InputTransferTask* createInputTransferTask( TransferRequester& requester,
                                                        unsigned           count ) {
        return new InputTransferTaskImpl(requester,count,*this);
    }

    virtual MessageGlossary& getGlossary() { return glossary_; }

    virtual void setRecordAtInit(const InputRegionRecord& ro, msgid_type maxMsgId);

    virtual void postInit( std::vector<regionid_type>& regs );

    virtual size_t rollOver();

private:
    void dispatchMessages( MsgIter begin, MsgIter end, std::vector<regionid_type>& regs);

    /// invoked from transfertask
    void doTransfer( TransferRequester& req, size_t count );

    void getRecord(InputRegionRecord& ro, msgid_type minRlast = 0);
    void setRecord(InputRegionRecord& ro, msgid_type maxMsgId = 0);
    void doSetRecord( RecordList::iterator to, const InputRegionRecord& from );
    std::string makeFilePath(regionid_type regId,uint32_t fn) const;

    inline dlvid_type getDlvId() const { return activityLog_->getDlvId(); }
    msgid_type getMinRlast();

private:
    smsc::logger::Logger*                      log_;
    DeliveryActivator&                         core_;
    smsc::core::synchronization::Mutex         wlock_;
    smsc::core::synchronization::Mutex         lock_;  // to add new regions
    RecordList                                 recordList_;
    RecordHash                                 recordHash_;
    RecordList::iterator                       rollingIter_;
    InputJournal&                              jnl_;
    uint32_t                                   lastfn_;
    ActivityLog*                               activityLog_; // not owned
    msgid_type                                 lastMsgId_;
    MessageGlossary                            glossary_;

    BlackList*                                 blackList_; // owned
    // smsc::core::synchronization::Mutex         dropLock_;
    // DropMsgHash                                dropMsgHash_;
    // drop file offset: -1 - no file, 0 - at start.
    // size_t                                     dropFileOffset_;
};

} // informer
} // smsc

#endif
