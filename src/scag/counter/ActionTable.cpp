#include "ActionTable.h"
#include "Manager.h"

namespace {
smsc::core::synchronization::Mutex logMutex;
}

namespace scag2 {
namespace counter {

smsc::logger::Logger* ActionTable::log_ = 0;

ActionTable::ActionTable( ActionList* list ) :
ref_(0),
actions_(list)
{
    if (!log_) {
        MutexGuard mg(logMutex);
        if (!log_) log_ = smsc::logger::Logger::getInstance("cnt.atbl");
    }
    smsc_log_debug(log_,"ctor %p",this);
}


ActionTable::~ActionTable()
{
    smsc_log_debug(log_,"dtor %p",this);
    for ( std::list<OldList>::iterator i = oldlists_.begin();
          i != oldlists_.end(); ++i ) {
        delete i->second;
    }
}


void ActionTable::modified( const char* cname, CntSeverity& sev, int64_t value, unsigned maxval )
{
    smsc_log_debug(log_,"%p: counter '%s' modified, new val=%llu",this,cname,value);
    ActionList* ptr;
    {
        MutexGuard mg(actlock_);
        ptr = actions_;
    }
    if ( ptr && ptr->list && maxval!=unsigned(-1) ) {
        const int64_t scaled = maxval!=0 ? value*100/maxval : value;
        const ActionLimit* list = ptr->list;
        for ( size_t i = 0; i < ptr->size; ++i ) {
            const ActionLimit& a = list[i];
            if ( a.compare(scaled) ) {
                Manager::getInstance().notify(cname,sev,value,a);
                return;
            }
        } // for
    }
    if ( sev > SEVNORMAL ) {
        // we have to reset severity here
        Manager::getInstance().notify(cname,sev,value,ActionLimit(0,OPTYPEGE,SEVNORMAL));
    }
}


void ActionTable::ref(bool add)
{
    bool destroy;
    {
        MutexGuard mg(lock_);
        if (add) {
            ++ref_;
            smsc_log_debug(log_,"%p: ref +1 => %u",this,ref_);
            return;
        }
        destroy = !--ref_;
        smsc_log_debug(log_,"%p: ref -1 => %u%s",this,ref_,destroy?" (need dtor)":"");
    }
    if (destroy) delete this;
}


void ActionTable::setNewActions( ActionList* newlist )
{
    const time_t t = time(0);
    MutexGuard mg(actlock_);
    if (actions_) oldlists_.push_back( std::make_pair(t,actions_) );
    actions_ = newlist;
    const time_t killt = t-5;
    for ( std::list<OldList>::iterator i = oldlists_.begin();
          i != oldlists_.end(); ) {
        if ( i->first >= killt ) break;
        delete i->second;
        i = oldlists_.erase(i);
    }
}

}
}
