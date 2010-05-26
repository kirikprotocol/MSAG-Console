#include <cstdio>
#include <memory>
#include <cassert>

#include "MessageCache.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "util/TimeSource.h"
#include "TaskInfo.h"
#include "StoreLog.h"
#include "logger/Logger.h"

using namespace scag2::prototypes::infosme;

const unsigned REGIONS = 10;
unsigned regionSent[REGIONS] = {0,};
unsigned regionSpeed[REGIONS] = {0,};


class NewMessageStorage : public MessageSource
{
private:
    class GenTask : public MessageUploadTask
    {
    public:
        GenTask( NewMessageStorage&      storage,
                 MessageUploadRequester& req,
                 taskid_type             taskId,
                 regionid_type           regionId,
                 unsigned                count ) :
        MessageUploadTask(req),
        storage_(&storage),
        taskId_(taskId), regionId_(regionId), count_(count)
        {
            if (!log_) {
                log_ = smsc::logger::Logger::getInstance("uptask");
            }
        }

        virtual int Execute()
        {
            smsc_log_debug(log_,"upload task %u/%u started to add %u messages",taskId_,regionId_,count_);
            MessageList ml;
            for ( unsigned i = 0; i < count_; ++i ) {
                MessageData md;
                Message& msg = md.msg;
                msg.subscriber = 9137654079ULL;
                msg.msgId = storage_->getNextMsgId();
                msg.textId = 1;
                char buf[20];
                sprintf(buf,"%u",msg.msgId);
                msg.userData = buf;
                msg.state = MsgState::input;
                smsc_log_debug(log_,"message %u/%u/%u prepared",taskId_,regionId_,msg.msgId);
                ml.push_back( md );
            }
            smsc_log_debug(log_,"adding new messages to storage...");
            requester_->addNewMessages(getCurrentTime(),ml,ml.begin(),ml.end());
            smsc_log_debug(log_,"upload task %u/%u finished",taskId_,regionId_);
            return 0;
        }

    private:
        static smsc::logger::Logger*  log_;

    private:
        NewMessageStorage* storage_;
        taskid_type        taskId_;
        regionid_type      regionId_;
        unsigned           count_;
    };

public:
    NewMessageStorage() : msgid_(0)
    {
    }

    virtual void requestNewMessages( MessageUploadRequester& req,
                                     taskid_type             taskId,
                                     regionid_type           regionId,
                                     unsigned                count )
    {
        MessageUploadTask* mut = new GenTask(*this,req,taskId,regionId,count);
        pool_.startTask( mut );
    }

    msgid_type getNextMsgId() {
        MutexGuard mg(seqMutex_);
        return ++msgid_;
    }


private:
    smsc::core::threads::ThreadPool pool_;
    smsc::core::synchronization::Mutex seqMutex_;
    msgid_type msgid_;
};


smsc::logger::Logger* NewMessageStorage::GenTask::log_ = 0;


MessageCache* makeStore( const TaskInfo& taskInfo,
                         StoreLog&       storeLog,
                         MessageSource&  messageSource )
{
    MessageCache* cache = new MessageCache(taskInfo);
    for ( regionid_type i = 0; i < REGIONS; ++i ) {
        RegionalStoragePtr ptr( new RegionalStorage(taskInfo, i,
                                                    storeLog, messageSource) );
        cache->addRegionalStorage( ptr );
    }
    return cache;
}


int getReadyRegion( unsigned deltaTime, int regionId )
{
    if ( regionId < 0 ) regionId = 0;
    const int inputRegionId = regionId;
    while ( regionSent[regionId]/regionSpeed[regionId] > deltaTime ) {
        ++regionId;
        if ( unsigned(regionId) >= REGIONS ) regionId = 0;
        if ( inputRegionId == regionId ) return -1;
    }
    return regionId;
}


bool processRegion( time_t currentTime, int regionId )
{
    typedef smsc::util::TimeSourceSetup::AbsUSec AbsUSec;
    const AbsUSec::usec_type usec = AbsUSec::getUSec();
    if ( usec & 0x4 ) {
        return false;
    }
    regionSent[regionId] += 1;
    return true;
}

int main()
{
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );
    smsc::logger::Logger* logmain = smsc::logger::Logger::getInstance("main");

    initgmt20100101();

    smsc::core::synchronization::EventMonitor mon;
    smsc::core::synchronization::MutexGuard mg(mon);

    typedef smsc::util::TimeSourceSetup::AbsSec AbsSec;

    // initializing regions
    const unsigned commonSpeedFactor = 10;
    for ( unsigned i = 0; i < REGIONS; ++i ) {
        regionSpeed[i] = commonSpeedFactor / ((i % 10)+1);
        regionSent[i] = 0;
    }

    TaskInfo taskInfo(15);
    smsc_log_info(logmain,"taskinfo created");

    StoreLog storeLog("./storelog.bin");
    smsc_log_info(logmain,"storelog created");

    NewMessageStorage messageSource;

    std::auto_ptr<MessageCache> store( makeStore(taskInfo,storeLog,messageSource) );
    smsc_log_info(logmain,"messagecache created");

    time_t startTime = AbsSec::getSeconds();
    int regionId = 1;
    for ( unsigned i = 0; i < 200; ++i ) {
        // get regionId
        const time_t currentTime = AbsSec::getSeconds();
        const unsigned deltaTime = currentTime - startTime;
        regionId = getReadyRegion(deltaTime,regionId);
        if ( regionId < 0 ) {
            // not ready
            mon.wait(100);
            continue;
        }
        if ( i % 100 == 0 ) {
            smsc_log_debug(logmain,"ready region is found: %u",unsigned(regionId));
        }
        Message msg;
        RegionalStoragePtr ptr = store->getRegionalStorage(regionId);
        if (!ptr.get()) {
            smsc_log_debug(logmain,"region %u is not found in storage",regionId);
            continue;
        }

        if ( ! ptr->getNextMessage( currentTime, msg ) ) {
            smsc_log_debug(logmain,"msg is not found in %u/%u",
                           ptr->getTaskInfo().getTaskId(),
                           ptr->getRegionId() );
            regionSent[regionId] += 1;
        } else {
            // message has been read from storage
            uint8_t state = 0;
            if ( !processRegion(currentTime,regionId) ) {
                smsc_log_info(logmain,"msg %u/%u/%u failed");
                state = MsgState::failed;
            } else {
                // processed successfully
                smsc_log_info(logmain,"msg %u/%u/%u processed");
                state = MsgState::sent;
            }
            // store->
            // workLog.changeState(pos,regionId,msg,state);
        }
    }
    return 0;
}
