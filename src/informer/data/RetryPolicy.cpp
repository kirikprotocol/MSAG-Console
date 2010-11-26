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
    SmppHash smppHash;
    PolicyList policyList;

    if (cfg) {
        std::auto_ptr<CStrSet> temps(cfg->getChildStrParamNames("temporary"));
        std::auto_ptr<Config> subc(cfg->getSubConfig("temporary",true));
        for ( CStrSet::const_iterator i = temps->begin(); i != temps->end(); ++i ) {
            std::auto_ptr<RetryString> rs;
            rs->init(i->c_str());
            policyList.push_back(rs.release());
            const char* items = subc->getString(i->c_str());
            const char* p = items;
            for (;;) {
                int smppCode = nextItem(p,i->c_str(),items,&smppHash);
                if (!smppCode) { break; }
                smppHash.Insert(smppCode,policyList.back());
            }
        }

        // reading immediate items
        try {
            const char* items = cfg->getString("immediate");
            const char* p = items;
            for (;;) {
                int smppCode = nextItem(p,"immediate",items,&smppHash);
                if (!smppCode) { break; }
                smppHash.Insert(smppCode,0);
            }
        } catch ( HashInvalidKeyException& ) {}

        // reading permanent items,
        // here we only check that codes are different from
        // those in immediate/temporary.
        try {
            const char* items = cfg->getString("permanent");
            const char* p = items;
            for (;;) {
                int smppCode = nextItem(p,"permanent",items,&smppHash);
                if (!smppCode) {break;}
                // do nothing here
            }
        } catch ( HashInvalidKeyException& ) {}
    }

    // adding default temporary/immediate codes
    {
        std::auto_ptr<RetryString> rs(new RetryString);
        rs->init("1h:*");
        policyList.push_back(rs.release());
        for ( int smppCode = 1; smppCode < int(sizeof(smsc::system::Status::errorStatus)/
                                               sizeof(smsc::system::Status::errorStatus[0]));
              ++smppCode ) {
            if ( smsc::system::Status::isErrorPermanent(smppCode) ) {
                // permanent
                continue;
            }
            RetryString** ptr = smppHash.GetPtr(smppCode);
            if ( ptr ) {
                // already forced by explicit policy
                continue;
            }
            // we have predefined list of immediate errors
            if ( smppCode == smsc::system::Status::MSGQFUL ||
                 smppCode == smsc::system::Status::THROTTLED ||
                 smppCode == smsc::system::Status::LICENSELIMITREJECT ) {
                smppHash.Insert(smppCode,0);
            } else {
                smppHash.Insert(smppCode,policyList.back());
            }
        }
    }

    if (cfg) {
        // now, cleaning permanent items
        try {
            const char* items = cfg->getString("permanent");
            const char* p = items;
            for (;;) {
                int smppCode = nextItem(p,"permanent",items,0);
                if (!smppCode) {break;}
                smppHash.Delete(smppCode);
            }
        } catch ( HashInvalidKeyException& ) {}
    }

    // switching
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        std::swap(policyList_,policyList);
        smppHash_ = smppHash;
    }
    for ( PolicyList::iterator i = policyList.begin(); i != policyList.end(); ++i ) {
        delete *i;
    }
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
                           const char*  policy,
                           const char*  items,
                           SmppHash*    smppHash )
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
    if (smppHash) {
        RetryString** ptr = smppHash->GetPtr(smppCode);
        if (ptr) {
            throw InfosmeException(EXC_CONFIG,"policy '%s' lists already used code %d",
                                   policy, smppCode );
        }
    }
    return smppCode;
}

}
}
