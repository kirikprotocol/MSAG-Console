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

void DeliveryInfo::read()
{
    smsc::core::buffers::TmpBuf<char,200> buf;
    const std::string& path = cs_.getStorePath();
    buf.setSize(path.size()+70);
    strcpy(buf.get(),path.c_str());
    assert(*(buf.get()+path.size()-1) == '/');
    char* end = makeDeliveryPath(dlvId_,buf.get()+path.size());
    smsc_log_debug(log_,"FIXME: reading D=%u info '%s'",dlvId_,buf.get());
    // reading state
    std::vector< std::string > files;
    files.reserve(2);
    makeDirListing(StateFileNameFilter(state_),S_IFREG).list(buf.get(),files);
    if (files.size()==1) {
        // ok
    } else {
        smsc_log_warn(log_,"wrong number of state files in D=%u: %u",dlvId_,unsigned(files.size()));
        // unlink all states
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
    }
}

}
}
