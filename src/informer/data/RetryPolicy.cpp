#include "RetryPolicy.h"
#include "system/status.h"
#include "util/config/Config.h"
#include "informer/io/InfosmeException.h"

using namespace smsc::util::config;

namespace eyeline {
namespace informer {

RetryPolicy::~RetryPolicy()
{
    for ( PolicyList::iterator i = policyList_.begin(); i != policyList_.end(); ++i ) {
        delete *i;
    }
}


void RetryPolicy::init( Config* cfg )
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    if (!cfg) {
        throw InfosmeException(EXC_LOGICERROR,"policy cfg is not supplied");
    }
    std::auto_ptr<CStrSet> temps(cfg->getChildStrParamNames("temporary"));
    std::auto_ptr<Config> subc(cfg->getSubConfig("temporary",true));
    for ( CStrSet::const_iterator i = temps->begin(); i != temps->end(); ++i ) {
        std::auto_ptr<RetryString> rs;
        rs->init(i->c_str());
        policyList_.push_back(rs.release());
        const char* items = subc->getString(i->c_str());
        const char* p = items;
        for (;;) {
            int smppCode = nextItem(p,i->c_str(),items);
            if (!smppCode) { break; }
            smppHash_.Insert(smppCode,policyList_.back());
        }
    }
    // reading immediate items
    try {
        const char* items = cfg->getString("immediate");
        const char* p = items;
        for (;;) {
            int smppCode = nextItem(p,"immediate",items);
            if (!smppCode) { break; }
            smppHash_.Insert(smppCode,0);
        }
    } catch ( HashInvalidKeyException& ) {}
    try {
        const char* items = cfg->getString("permanent");
        const char* p = items;
        for (;;) {
            int smppCode = nextItem(p,"permanent",items);
            if (!smppCode) {break;}
            // do nothing, probably later we will have default codes
        }
    } catch ( HashInvalidKeyException& ) {}
}


timediff_type RetryPolicy::getRetryInterval( bool               /* not used */,
                                             int                 smppState,
                                             uint16_t            retryCount ) const
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    RetryString** ptr = smppHash_.GetPtr(smppState);
    if (!ptr) return -1; // permanent
    if (!*ptr) return 0; // immediate
    return (*ptr)->getRetryInterval( retryCount );
}


int RetryPolicy::nextItem( const char*& p,
                           const char* policy,
                           const char* items )
{
    while ( *p == ' ' ) ++p;
    if (*p == '\0') return 0;

    int shift = 0;
    int smppCode;
    if ( -1 == sscanf(p,"%d%n",&smppCode,&shift) ) {
        throw InfosmeException(EXC_CONFIG,"policy '%s' has invalid code list '%s'",
                               policy, items );
    }
    if ( smppCode == smsc::system::Status::OK ) {
        throw InfosmeException(EXC_CONFIG,"policy '%s' lists code 0",policy);
    }
    p += shift;
    while ( *p == ' ' ) ++p;
    if (*p == ',') {
        ++p;
        while ( *p == ' ' ) ++p;
        if (*p == '\0') {
            throw InfosmeException(EXC_CONFIG,"policy '%s' has invalid format '%s'",policy,items);
        }
    } 

    // check the code
    RetryString** ptr = smppHash_.GetPtr(smppCode);
    if (ptr) {
        throw InfosmeException(EXC_CONFIG,"policy '%s' lists already used code %d",
                               policy, smppCode );
    }
    return smppCode;
}

}
}
