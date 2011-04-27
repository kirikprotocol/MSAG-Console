#include <algorithm>
#include "ProfileBackup.h"
#include "ProfileLog.h"
#include "util/PtrDestroy.h"
#include "scag/pvss/profile/Profile.h"
#include "logger/Logger.h"

namespace {
smsc::core::synchronization::Mutex mtx;
}

namespace scag2 {
namespace pvss {

smsc::logger::Logger* ProfileBackup::log_ = 0;

ProfileBackup::ProfileBackup( ProfileLog* dblog ) :
dblog_(dblog)
{
    if (!log_) {
        MutexGuard mg(mtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("pvss.pbck");
    }
}


ProfileBackup::~ProfileBackup() {
    cleanup();
    // not needed, destroyed from profilelogroller
    // delete dblog_;
}


void ProfileBackup::addProperty( const Property& prop, bool logOnly )
{
    if (!logOnly) {
        operationList_.push_back( PvssOp(unsigned(fixedPolicies_.size()),PROPADD) );
        fixedPolicies_.push_back( PropTime(prop.getName()) );
    }
    addLogMsg(PROPADD, prop.toString());
}


void ProfileBackup::getProperty( const Property& prop )
{
    addLogMsg(PROPGET,prop.getName());
}


void ProfileBackup::fixProperty( const Property& prop )
{
    operationList_.push_back( PvssOp(unsigned(modifiedProperties_.size()),PROPMOD) );
    modifiedProperties_.push_back(prop);
}


void ProfileBackup::propertyUpdated( const Property& prop )
{
    addLogMsg(PROPMOD, prop.toString());
}


void ProfileBackup::delProperty( Property* prop, bool logOnly )
{
    if (!logOnly) {
        operationList_.push_back( PvssOp(unsigned(deletedProperties_.size()),PROPDEL) );
        deletedProperties_.push_back( prop );
    }
    addLogMsg(PROPDEL,prop->getName());
}


void ProfileBackup::expireProperty( const char* profkey, const Property& prop )
{
    smsc_log_info(dblog_,"E key=%s name=%s", profkey, prop.getName());
}


void ProfileBackup::fixTimePolicy( const Property& prop )
{
    operationList_.push_back( PvssOp(unsigned(fixedPolicies_.size()),PROPFIX) );
    fixedPolicies_.push_back( PropTime(prop) );
    // FIXME: should we log?
}


void ProfileBackup::cleanup()
{
    operationList_.clear();
    std::for_each( deletedProperties_.begin(),
                   deletedProperties_.end(),
                   smsc::util::PtrDestroy() );
    deletedProperties_.clear();
    modifiedProperties_.clear();
    fixedPolicies_.clear();
    jlen_ = 0;
}


void ProfileBackup::rollback( Profile& prof )
{
    smsc_log_debug(log_,"backing up profile %p key=%s",&prof,prof.getKey().c_str());
    for ( std::vector< PvssOp >::reverse_iterator i = operationList_.rbegin();
          i != operationList_.rend();
          ++i ) {
        switch ( i->opType ) {
        case PROPADD : {
            const PropTime& p = fixedPolicies_[i->position];
            prof.DeleteProperty(p.name.c_str());
            smsc_log_debug(log_,"delete %p name=%s",&prof,p.name.c_str());
            break;
        }
        case PROPDEL : {
            Property* p = deletedProperties_[i->position];
            prof.AddProperty(*p);
            smsc_log_debug(log_,"add %p %s",&prof,p->toString().c_str());
            delete p;
            deletedProperties_[i->position] = 0;
            break;
        }
        case PROPMOD : {
            const Property& p = modifiedProperties_[i->position];
            Property* ptr = prof.GetProperty(p.getName());
            if (ptr) {
                smsc_log_debug(log_,"mod %p %s -> %s",
                               &prof,ptr->toString().c_str(),p.toString().c_str());
                *ptr = p;
            } else {
                prof.AddProperty(p);
                smsc_log_debug(log_,"mod(add) %p %s",&prof,p.toString().c_str());
            }
            break;
        }
        case PROPFIX : {
            const PropTime& p = fixedPolicies_[i->position];
            Property* ptr = prof.GetProperty(p.name.c_str());
            if (ptr) {
                ptr->setTimePolicy(p.timePolicy,p.finalDate,p.lifeTime);
                smsc_log_debug(log_,"time %p %s",&prof,ptr->toString().c_str());
            } else {
                smsc_log_debug(log_,"time %p propnotfound",&prof);
            }
            break;
        }
        default : 
            break;
        } // switch
    } // for
    cleanup();
}


void ProfileBackup::flushLogs( const Profile& prof )
{
    if ( ! dblog_->isInfoEnabled() ) return;
    const char* profkey = prof.getKey().c_str();
    for ( size_t i = 0; i < jlen_; ++i ) {
        LogMsg& msg = journal_[i];
        smsc::logger::Logger::LogLevel level = smsc::logger::Logger::LEVEL_INFO;
        char what;
        const char* mid;
        switch (PvssOpType(msg.opType)) {
        case PROPADD : {
            what = 'A';
            mid = "property";
            break;
        }
        case PROPDEL : {
            what = 'D';
            mid = "name";
            break;
        }
        case PROPMOD : {
            what = 'U';
            mid = "property";
            break;
        }
        case PROPFIX : {
            what = 'E';
            mid = "name";
            break;
        }
        case PROPGET : {
            what = 'G';
            mid = "name";
            level = smsc::logger::Logger::LEVEL_DEBUG;
            break;
        }
        } // switch
        if ( dblog_->isLogLevelEnabled(level) ) {
            dblog_->log(level,"%c key=%s %s=%s",what,profkey,mid,msg.msg.c_str());
        }
    }
    jlen_ = 0;
}

}
}
