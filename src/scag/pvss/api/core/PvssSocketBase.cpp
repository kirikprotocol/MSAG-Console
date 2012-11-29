#include "PvssSocketBase.h"

using namespace smsc::core::synchronization;
using smsc::logger::Logger;

namespace scag2 {
namespace pvss {
namespace core {

namespace {
Mutex logMutex;
}

smsc::logger::Logger* PvssSocketBase::log_ = 0;
unsigned long long    PvssSocketBase::total_ = 0;

void PvssSocketBase::initHostName( const char* hn )
{
    if (!log_) {
        MutexGuard mg(logMutex);
        if (!log_) log_ = Logger::getInstance("pvss.sock");
    }
    if ( sock_ && host_.empty() ) {
        char buf[60];
        if ( hn ) {
        } else if ( sock_->isConnected() ) {
            sock_->GetPeer(buf);
            hn = buf;
        } else {
            return;
        }
        const char* colon = strchr(const_cast<char*>(hn),':');
        if ( colon ) {
            host_ = std::string(hn,colon-hn);
            port_ = atoi(colon+1);
        }
    }
}

}
}
}
