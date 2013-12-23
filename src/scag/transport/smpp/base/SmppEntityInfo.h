#ifndef __SCAG_TRANSPORT_SMPP_BASE_SMPPENTITYINFO_H
#define __SCAG_TRANSPORT_SMPP_BASE_SMPPENTITYINFO_H

#include "sms/sms_const.h"
#include "scag/transport/smpp/SmppTypes.h"
#include "core/buffers/FixedLengthString.hpp"

namespace scag2 {
namespace transport {
namespace smpp {

using smsc::core::buffers::FixedLengthString;

struct SmppEntityInfo 
{
//    FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1>  systemId;
//    FixedLengthString<smsc::sms::MAX_SMEPASSWD_TYPE_LENGTH+1> password;
//    FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1>  bindSystemId;
//    FixedLengthString<smsc::sms::MAX_SMEPASSWD_TYPE_LENGTH+1> bindPassword;
//    FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> metaGroupId;
    SmppEntityType type;
    smsc::sms::SmeSystemIdType systemId;
    smsc::sms::SmePasswordType password;
    smsc::sms::SmeSystemIdType bindSystemId;
    smsc::sms::SmePasswordType bindPassword;
    FixedLengthString<42> addressRange;
    FixedLengthString<13> systemType;
    smsc::sms::SmeSystemIdType metaGroupId;
    int timeOut;
    SmppBindType bindType;
    FixedLengthString<32> host;
    int  port;
    FixedLengthString<32> altHost;
    int  altPort;
    uint8_t uid;
    int sendLimit;
    int inQueueLimit;
    int outQueueLimit;
    bool enabled;
    bool snmpTracking;    // watch on this sme via snmp
    bool defaultLatin1;   //11443 : flag to change dc 0->3

    SmppEntityInfo()
    {
        type=etUnknown;
        timeOut=0;
        bindType=btNone;
        port=0;
        altPort=0;
        uid=0;
        sendLimit=0;
        inQueueLimit=0;
        outQueueLimit=0;
        enabled=false;
        snmpTracking=true;
        defaultLatin1=false;
    }
};

enum BalancingPolicy{
    bpRoundRobin,bpRandom
};
enum MetaEntityType{
    mtMetaService,mtMetaSmsc
};

struct MetaEntityInfo
{
    MetaEntityInfo()
    {
        type=mtMetaService;
        policy=bpRoundRobin;
        persistanceEnabled=true;
    }
//    FixedLengthString<smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1> systemId;
    smsc::sms::SmeSystemIdType systemId;
    MetaEntityType type;
    BalancingPolicy policy;
    bool persistanceEnabled;
};

struct SmppEntityAdminInfo
{
    std::string systemId;
    SmppBindType bindType;
    std::string host;
    int  port;
    bool connected;
};

}//smpp
}//transport
}//scag

#endif /* !__SCAG_TRANSPORT_SMPP_BASE_SMPPENTITYINFO_H */
