#include <memory>
#include "scag/pvss/common/ScopeType.h"
#include "logger/Logger.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/data/Property.h"

using namespace scag2::pvss;
using scag2::pvss::ScopeType;

int main()
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* log_ = smsc::logger::Logger::getInstance("bck.proc");

    ScopeType scope_ = SCOPE_ABONENT;
    std::string line = "I 27-05-2009 03:15:02,579 024  pvss.abnt: A key=.1.1.79156062209 property=\"lovemob.inviter.gift.79882600745\" INT: 255 TIME_POLICY: R_ACCESS FINAL_DATE: 2009/07/28 06:31:01 LIFE_TIME: 7776000";
    // std::string line = "I 26-07-2009 00:04:41,923 039  pvss.serv: A key=184 property=\"4205\" STRING: \"http://m1.eyeline.mobi/a79188049413/wap.watago.mobi/sd2/4205/mg/wap/loc/\" TIME_POLICY: ACCESS FINAL_DATE: 2009/07/25 17:34:41 LIFE_TIME: 1800";

    std::auto_ptr<char> prop;
    size_t proplen = 0;
    char logLevel, act;
    unsigned day, month, year, hour, minute, second, msec, thread;
    int ikey;
    char cat[30], skey[100];
    {
        size_t ls = line.size()+1;
        if ( proplen < ls ) {
            prop.reset( new char[ls] );
            proplen = ls;
        }
    }
    int sc;
    if ( scope_ == SCOPE_ABONENT ) {
        sc = sscanf(line.c_str(), 
                    "%c %02u-%02u-%04u %02u:%02u:%02u,%03u %u %s %c key=%s property=%[^\n]",
                    &logLevel,&day,&month,&year,&hour,&minute,&second,&msec,&thread,
                    cat, &act, skey, prop.get() );
    } else {
        sc = sscanf(line.c_str(), 
                    "%c %02u-%02u-%04u %02u:%02u:%02u,%03u %u %s %c key=%u property=%[^\n]",
                    &logLevel,&day,&month,&year,&hour,&minute,&second,&msec,&thread,
                    cat, &act, &ikey, prop.get() );
    }
    if ( sc <= 0 ) {
        smsc_log_warn(log_,"cannot sscanf(%s): %d",line.c_str(),sc);
    } else {
        smsc_log_debug(log_,"sc   = %d",sc);
        smsc_log_debug(log_,"cat  = %s",cat);
        smsc_log_debug(log_,"prop = %s",prop.get());

        ProfileKey profileKey;
        switch (scope_) {
        case (SCOPE_ABONENT) : profileKey.setAbonentKey(skey); break;
        case (SCOPE_SERVICE) : profileKey.setServiceKey(ikey); break;
        case (SCOPE_OPERATOR) : profileKey.setOperatorKey(ikey); break;
        case (SCOPE_PROVIDER) : profileKey.setProviderKey(ikey); break;
        default: throw smsc::util::Exception("cannot set profile key of unknown scope %u", scope_);
        }

        Property property;
        try {
            property.fromString(prop.get());
            smsc_log_debug(log_,"key: %s, property: %s",
                           profileKey.toString().c_str(),
                           property.toString().c_str());
        } catch (...) {
            smsc_log_warn(log_,"cannot parse property");
        }
    }
    return 0;
}
