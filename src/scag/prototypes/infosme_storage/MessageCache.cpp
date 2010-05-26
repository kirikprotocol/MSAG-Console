#include <vector>
#include "MessageCache.h"
#include "RegionalStorage.h"
#include "util/Exception.hpp"
#include "TaskInfo.h"

using namespace smsc::core::synchronization;

namespace scag2 {
namespace prototypes {
namespace infosme {

/*
bool MessageCache::hasMessages( bool onlyNew )
{
    MutexGuard mg(cacheLock_);
    if (!onlyNew) {
        if (cache_.Count() > 0) return true;
    }
    for ( unsigned i = currentFile_; i < unsigned(inputFiles_.Count()); ++i ) {
        if (inputFiles_[i] && inputFiles_[i]->hasMessages()) return true;
    }
    return false;
}


bool MessageCache::getMessage( msgid_type msgId, Message& msg )
{
    MutexGuard mg(cacheLock_);
    PosMessage* ptr = cache_.GetPtr(msgId);
    if (!ptr) return false;
    msg = ptr->msg;
    return true;
}


bool MessageCache::getNextMessage( time_t currentTime, Message& msg )
{
    RelockMutexGuard mg(cacheLock_);
    if ( ! resendQueue_.empty() ) {
        std::multimap< time_t, PosMessage >::iterator i = resendQueue_.begin();
        if ( i->first <= currentTime ) {
            PosMessage pm = i->second;
            // FIXME: adjust retry time to a diff of i->first - currentTime
            cache_.Insert(msg.msgId,pm);
            resendQueue_.erase(i);
            mg.Unlock();
            msg = pm.msg;
            if (alog_) alog_->messageInProcess(currentTime,regionId_,pm);
            return true;
        }
    }
    // redirect to the input files
    for ( ; currentFile_ < unsigned(inputFiles_.Count()); ++currentFile_ ) {
        InputMessageFile* inputFile = inputFiles_[currentFile_];
        if (!inputFile) continue;
        PosMessage pm;
        const int res = inputFile->getNextMessage(pm);
        if (res > 0) {
            // message has been read
            pm.msg.timeLeft = validityTime_;
            cache_.Insert(pm.msg.msgId,pm);
            mg.Unlock();
            if (alog_) alog_->messageInProcess(currentTime,regionId_,pm);
            return true;
        } else if ( res == -1 ) {
            // file is not ready yet
            return false;
        }
    }
    if (currentFile_>0) --currentFile_;
    return false;
}


void MessageCache::messageSent( msgid_type msgId,
                                time_t currentTime,
                                const char* receipt )
{
    RelockMutexGuard mg(cacheLock_);
    PosMessage* ptr = cache_.GetPtr(msgId);
    if (!ptr) throw smsc::util::Exception("message %u is not found",msgId);
    PosMessage pm(*ptr);
    mg.Unlock();
    alog_->messageSent(currentTime,regionId_,pm,receipt);
}


void MessageCache::retryMessage( msgid_type msgId,
                                 time_t currentTime,
                                 uint32_t retryTime,
                                 int smppState )
{
    RelockMutexGuard mg(cacheLock_);
    PosMessage pm;
    if ( !cache_.Pop(msgId,pm) ) throw smsc::util::Exception("message is not found: %u",msgId);
    if ( pm.msg.timeLeft > minRetryTime_ ) {
        if ( retryTime > pm.msg.timeLeft ) retryTime = pm.msg.timeLeft;
        pm.msg.timeLeft -= retryTime;
        resendQueue_.insert(std::make_pair(currentTime+retryTime,pm));
        mg.Unlock();
        if (alog_) alog_->retryMessage(currentTime,regionId_,pm,retryTime,smppState);
    } else {
        // cannot retry
        mg.Unlock();
        if (alog_) alog_->finalizeMessage(currentTime,regionId_,pm,MsgState::expired,smppState);
    }
}


void MessageCache::finalizeMessage( msgid_type msgId,
                                    time_t     currentTime,
                                    uint8_t    state,
                                    int        smppState )
{
    RelockMutexGuard mg(cacheLock_);
    PosMessage pm;
    if (!cache_.Pop(msgId,pm)) throw smsc::util::Exception("message is not found: %u",msgId);
    mg.Unlock();
    if (alog_) alog_->finalizeMessage(currentTime,regionId_,pm,state,smppState);
}
 */


MessageCache::MessageCache( const TaskInfo& taskInfo ) :
taskInfo_(&taskInfo)
{
}


RegionalStoragePtr MessageCache::getRegionalStorage( regionid_type regionId )
{
    MutexGuard mg(cacheLock_);
    RegionalStoragePtr* ptr = storages_.GetPtr(regionId);
    if (!ptr) return RegionalStoragePtr();
    return *ptr;
}


void MessageCache::rollOver()
{
    std::vector<RegionalStoragePtr> ptrs;
    {
        MutexGuard mg(cacheLock_);
        ptrs.reserve(storages_.Count());
        int id;
        RegionalStoragePtr* ptr;
        for ( smsc::core::buffers::IntHash< RegionalStoragePtr >::Iterator i(storages_);
              i.Next(id,ptr); ) {
            if (ptr->get()) ptrs.push_back(*ptr);
        }
    }
    for ( std::vector< RegionalStoragePtr >::iterator i = ptrs.begin();
          i != ptrs.end();
          ++i ) {
        (*i)->rollOver();
    }
}


void MessageCache::addRegionalStorage( RegionalStoragePtr storage )
{
    if (!storage.get()) {
        throw smsc::util::Exception("MessageCache: storage to be added in task %u is NULL",
                                    taskInfo_->getTaskId() );
    }
    if ( &(storage->getTaskInfo()) != taskInfo_ ) {
        throw smsc::util::Exception("MessageCache: storage %u for task %u has different taskInfo",
                                    storage->getRegionId(), taskInfo_->getTaskId() );
    }

    MutexGuard mg(cacheLock_);
    RegionalStoragePtr* ptr = storages_.GetPtr(storage->getRegionId());
    if (ptr) {
        throw smsc::util::Exception("region %u already exists in task %u",
                                    storage->getRegionId(),
                                    taskInfo_->getTaskId() );
    }
    storages_.Insert(storage->getRegionId(),storage);
}

}
}
}
