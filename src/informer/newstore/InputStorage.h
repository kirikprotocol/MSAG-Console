#ifndef _INFORMER_INPUTSTORAGE_H
#define _INFORMER_INPUTSTORAGE_H

#include "informer/data/InputMessageSource.h"
#include "informer/data/MessageGlossary.h"
#include "informer/data/CommonSettings.h"
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
    class InputTransferTask : public TransferTask {
    public:
        InputTransferTask( TransferRequester& req,
                           unsigned count,
                           InputStorage& store ) :
        TransferTask(req,count), store_(store) {}
        virtual const char* taskName() { return "transtask"; }
        virtual int Execute() {
            store_.doTransfer(requester_,count_);
            return 0;
        }
    private:
        InputStorage& store_;
    };

public:
    InputStorage( InfosmeCore& core,
                  dlvid_type   dlvId,
                  InputJournal& journal );

    virtual ~InputStorage();

    virtual void addNewMessages( MsgIter begin, MsgIter end );

    virtual TransferTask* startTransferTask( TransferRequester& requester,
                                             unsigned           count,
                                             bool               mayDetachRegion );

    virtual MessageGlossary& getGlossary() { return glossary_; }

    virtual dlvid_type getDlvId() const {
        return dlvId_;
    }
    virtual const std::string& getStorePath() const {
        return jnl_.getCS().getStorePath();
    }

private:
    void dispatchMessages( MsgIter begin, MsgIter end, std::vector<regionid_type>& regs);

    /// invoked from transfertask
    void doTransfer( TransferRequester& req, unsigned count );

    void getRecord(regionid_type regid, InputRegionRecord& ro);
    void setRecord(regionid_type regid, InputRegionRecord& ro, msgid_type maxMsgId = 0);
    std::string makeFilePath(regionid_type regId,uint32_t fn) const;

private:
    smsc::logger::Logger* log_;
    InfosmeCore&          core_;
    smsc::core::synchronization::Mutex wlock_;
    smsc::core::synchronization::Mutex lock_;  // to add new regions
    smsc::core::buffers::IntHash< InputRegionRecord >  regions_;
    InputJournal&                              jnl_;
    uint32_t                                   lastfn_;
    dlvid_type                                 dlvId_;
    msgid_type                                 lastMsgId_;
    MessageGlossary                            glossary_;
};

} // informer
} // smsc

#endif
