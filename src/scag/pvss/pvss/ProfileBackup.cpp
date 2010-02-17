#include <algorithm>
#include "ProfileBackup.h"
#include "util/PtrDestroy.h"
#include "scag/pvss/profile/Profile.h"
#include "logger/Logger.h"

namespace {
smsc::core::synchronization::Mutex mtx;
}

namespace scag2 {
namespace pvss {

smsc::logger::Logger* ProfileBackup::log_ = 0;

ProfileBackup::ProfileBackup()
{
    if (!log_) {
        MutexGuard mg(mtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("pvss.pbck");
    }
}


void ProfileBackup::addProperty( const Property& prop )
{
    operationList_.push_back( PvssOp(unsigned(fixedPolicies_.size()),PROPADD) );
    fixedPolicies_.push_back( PropTime(prop.getName()) );
}


void ProfileBackup::delProperty( Property* prop )
{
    operationList_.push_back( PvssOp(unsigned(deletedProperties_.size()),PROPDEL) );
    deletedProperties_.push_back( prop );
}


void ProfileBackup::fixPolicy( Property& prop )
{
    operationList_.push_back( PvssOp(unsigned(fixedPolicies_.size()),PROPFIX) );
    fixedPolicies_.push_back( PropTime(prop.getName()) );
}


void ProfileBackup::fixProperty( Property& prop )
{
    operationList_.push_back( PvssOp(unsigned(modifiedProperties_.size()),PROPMOD) );
    modifiedProperties_.push_back(prop);
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
        case PROPFIX :
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
    }
}

}
}
