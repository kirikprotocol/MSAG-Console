#ifndef _INFORMER_INPUTSTORAGE_H
#define _INFORMER_INPUTSTORAGE_H

#include "informer/data/InputMessageSource.h"
#include "informer/data/MessageGlossary.h"
#include "informer/data/CommonSettings.h"
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
            store_.doTransfer(requester_,count_);
            return 0;
        }
    private:
        InputStorage& store_;
    };

public:
    InputStorage( InfosmeCore&  core,
                  InputJournal& journal );

    virtual ~InputStorage();

    virtual void init( ActivityLog& actLog );

    virtual void addNewMessages( MsgIter begin, MsgIter end );

    virtual InputTransferTask* startInputTransfer( TransferRequester& requester,
                                                   unsigned           count,
                                                   bool               mayDetachRegion );
    virtual void startResendTransfer( ResendTransferTask* task );

    virtual MessageGlossary& getGlossary() { return glossary_; }

    virtual InfosmeCore& getCore() { return core_; }

    virtual void setRecordAtInit(const InputRegionRecord& ro, msgid_type maxMsgId);

    virtual void postInit( std::vector<regionid_type>& regs );

    virtual size_t rollOver();

private:
    void dispatchMessages( MsgIter begin, MsgIter end, std::vector<regionid_type>& regs);

    /// invoked from transfertask
    void doTransfer( TransferRequester& req, unsigned count );

    void getRecord(InputRegionRecord& ro);
    void setRecord(InputRegionRecord& ro, msgid_type maxMsgId = 0);
    void doSetRecord( RecordList::iterator to, const InputRegionRecord& from );
    std::string makeFilePath(regionid_type regId,uint32_t fn) const;

    inline dlvid_type getDlvId() const { return activityLog_->getDlvId(); }

private:
    smsc::logger::Logger*                      log_;
    InfosmeCore&                               core_;
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
};

} // informer
} // smsc

#endif
