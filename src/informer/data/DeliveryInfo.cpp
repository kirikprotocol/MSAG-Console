#include <cassert>
#include <cstring>
#include <vector>
#include "informer/io/DirListing.h"
#include "informer/io/FileGuard.h"
#include "DeliveryInfo.h"
#include "CommonSettings.h"
#include "core/buffers/TmpBuf.hpp"

namespace {

using namespace eyeline::informer;

struct StateFileNameFilter
{
    StateFileNameFilter(DlvState& st) : state(st) {}
    bool operator() ( const char* fn ) const {
        for ( DlvState st = DLVSTATE_PAUSED; st <= DLVSTATE_MAX; st = DlvState(int(st)+1) ) {
            if ( 0 == strcmp(fn,dlvStateToString(st)) ) {
                state = st;
                return true;
            }
        }
        return false;
    }
    DlvState& state;
};

}

namespace eyeline {
namespace informer {

smsc::logger::Logger* DeliveryInfo::log_ = 0;


/*
void DeliveryInfo::incrementStats( const DeliveryStats& stats, DeliveryStats* result )
{
    MutexGuard mg(lock_);
    if ( stats.firstMessageSent ) {
        if ( !stats_.firstMessageSent ) {
            stats_.firstMessageSent = stats.firstMessageSent;
        }
        if ( stats_.lastMessageSent < stats.firstMessageSent ) {
            stats_.lastMessageSent = stats.firstMessageSent;
        }
    }
    stats_.totalMessages += stats.totalMessages;
    stats_.sentMessages += stats.sentMessages;
    stats_.dlvdMessages += stats.dlvdMessages;
    stats_.failedMessages += stats.failedMessages;
    stats_.expiredMessages += stats.expiredMessages;
    if (result) { *result = stats_; }
}


void DeliveryInfo::updateStats( const DeliveryStats& stats )
{
    MutexGuard mg(lock_);
    // find out the minimum first message sent time
    if ( stats.firstMessageSent ) {
        if ( stats_.firstMessageSent > stats.firstMessageSent ) {
            stats_.firstMessageSent = stats.firstMessageSent;
        }
    }
    if ( stats_.lastMessageSent < stats.lastMessageSent ) {
        stats_.lastMessageSent = stats.lastMessageSent;
    }
    if ( stats_.totalMessages < stats.totalMessages ) {
        stats_.totalMessages = stats.totalMessages;
    }
    if ( stats_.sentMessages < stats.sentMessages ) {
        stats_.sentMessages = stats.sentMessages;
    }
    if ( stats_.dlvdMessages < stats.dlvdMessages ) {
        stats_.dlvdMessages = stats.dlvdMessages;
    }
    if ( stats_.failedMessages < stats.failedMessages ) {
        stats_.failedMessages = stats.failedMessages;
    }
    if ( stats_.expiredMessages < stats.expiredMessages ) {
        stats_.expiredMessages = stats.expiredMessages;
    }
}


void DeliveryInfo::getStats( DeliveryStats& stats ) const
{
    MutexGuard mg(lock_);
    stats = stats_;
}
 */


void DeliveryInfo::read()
{
    smsc::core::buffers::TmpBuf<char,200> buf;
    const std::string& path = cs_.getStorePath();
    buf.setSize(path.size()+70);
    strcpy(buf.get(),path.c_str());
    assert(*(buf.get()+path.size()-1) == '/');
    // char* end = 
    makeDeliveryPath(dlvId_,buf.get()+path.size());
    smsc_log_debug(log_,"FIXME: reading D=%u info '%s'",dlvId_,buf.get());
    // reading state
    std::vector< std::string > files;
    files.reserve(2);
    makeDirListing(StateFileNameFilter(state_),S_IFREG).list(buf.get(),files);
    if (files.size()==1) {
        // ok
    } else {
        // FIXME: should we be non-invasive? I.e. do not touch delivery elements
        throw InfosmeException("D=%u has wrong number of state files: %u",dlvId_,unsigned(files.size()));
        // smsc_log_warn(log_,"wrong number of state files in D=%u: %u",dlvId_,unsigned(files.size()));
        // unlink all states
        /*
        for ( DlvState st = DLVSTATE_PAUSED; st <= DLVSTATE_MAX; st = DlvState(int(st)+1) ) {
            strcpy(end,dlvStateToString(st));
            unlink(buf.get()); // dont care about rc
        }
        *end = '\0';
        state_ = DLVSTATE_PAUSED;
        strcpy(end,dlvStateToString(state_));
        FileGuard fg;
        fg.create(buf.get());
        fg.close();
         */
    }
}

}
}
