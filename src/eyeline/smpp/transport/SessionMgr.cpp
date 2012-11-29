#include "SessionMgr.h"
#include "eyeline/smpp/SmppException.h"

using namespace smsc::core::synchronization;

namespace eyeline {
namespace smpp {


SessionMgr::SessionMgr() :
log_(smsc::logger::Logger::getInstance("smpp.sesmg"))
{
    smsc_log_info(log_,"ctor");
}


SessionMgr::~SessionMgr()
{
    smsc_log_info(log_,"dtor");
}


void SessionMgr::stop()
{
    smsc_log_debug(log_,"stopping...");
    MutexGuard mg(lock_);
    char* si;
    SessionBasePtr ptr;
    for ( SessionHash::Iterator i(&sessionHash_); i.Next(si,ptr); ) {
        ptr->destroy();
    }
    sessionHash_.Empty();
}


bool SessionMgr::getSession( const char* sId, SessionBasePtr& ptr )
{
    SessionBasePtr* p = sessionHash_.GetPtr(sId);
    if (!p) {
        ptr.reset(0);
        return false;
    }
    ptr = *p;
    return true;
}


void SessionMgr::addSession( SessionBase& ptr )
{
    // if (!ptr) throw SmppException("addSession exc: null ptr");
    smsc_log_debug(log_,"adding S'%s'",ptr.getSessionId());
    MutexGuard mg(lock_);
    SessionBasePtr* p = sessionHash_.GetPtr(ptr.getSessionId());
    if (p) {
        throw SmppException("addSession: S'%s' is already added",ptr.getSessionId());
    }
    p = sessionHash_.SetItem(ptr.getSessionId(),SessionBasePtr());
    p->reset(&ptr);
}


void SessionMgr::removeSession( const char* sid )
{
    if (!sid) throw SmppException("removeSession: null sid");
    smsc_log_debug(log_,"removing S'%s'",sid);
    MutexGuard mg(lock_);
    sessionHash_.Delete(sid);
}


}
}
