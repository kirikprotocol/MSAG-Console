#include "ActionTable.h"

namespace {
smsc::core::synchronization::Mutex logMutex;
}

namespace scag2 {
namespace counter {

smsc::logger::Logger* ActionTable::log_ = 0;

ActionTable::ActionTable() :
ref_(0)
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
}


void ActionTable::modified( Counter& counter, int64_t value )
{
    smsc_log_debug(log_,"%p: counter '%s' modified, new val=%llu",this,counter.getName().c_str(),value);
    ActionList* ptr;
    {
        MutexGuard mg(actlock_);
        ptr = actions_;
    }
    ActionParams* list = ptr->list;
    for ( size_t i = 0; i < ptr->size; ++i ) {
        const ActionParams& a = list[i];
        switch (a.optype) {
        case ActionParams::GT : {
            if ( value > a.limit ) {
                notify(counter,value,a);
                if (a.skip) i = ptr->size;
            }
            break;
        }
        case ActionParams::LT : {
            if ( value < a.limit ) {
                notify(counter,value,a);
                if (a.skip) i = ptr->size;
            }
        }
        } // switch
    } // for
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
    oldlists_.push_back( std::make_pair(t,actions_) );
    actions_ = newlist;
    const time_t killt = t-5;
    for ( std::list<OldList>::iterator i = oldlists_.begin();
          i != oldlists_.end(); ) {
        if ( i->first >= killt ) break;
        delete i->second;
        i = oldlists_.erase(i);
    }
}


void ActionTable::notify( Counter& c, int64_t value, const ActionParams& params )
{
    smsc_log_debug(log_,"value %lld of counter '%s' is %s than limit=%llu",
                   value, c.getName().c_str(),
                   params.optype == ActionParams::GT ? "greater" : "less",
                   params.limit );
}

}
}
