#ident "$Id$"
#ifndef __SMSC_INMAN_MAPATSI_COMPS_HPP__
#define __SMSC_INMAN_MAPATSI_COMPS_HPP__

//#include <string>

#include "inman/common/adrutil.hpp"
#include "logger/Logger.h"
#include "inman/comp/compdefs.hpp"

using smsc::logger::Logger;
using smsc::inman::comp::Component;

/* GVR NOTE: while linking the below enums are taken from generated *
 * asn1/c codec, so they should not have namespace prefix.          */
#ifndef	_RequestedCAMEL_SubscriptionInfo_H_
/* this is the clone of RequestedCAMEL-SubscriptionInfo.h::RequestedCAMEL_SubscriptionInfo_e */
typedef enum RequestedCAMEL_SubscriptionInfo {
    RequestedCAMEL_SubscriptionInfo_o_CSI       = 0,
    RequestedCAMEL_SubscriptionInfo_t_CSI       = 1,
    RequestedCAMEL_SubscriptionInfo_vt_CSI      = 2,
    RequestedCAMEL_SubscriptionInfo_tif_CSI     = 3,
    RequestedCAMEL_SubscriptionInfo_gprs_CSI    = 4,
    RequestedCAMEL_SubscriptionInfo_mo_sms_CSI  = 5,
    RequestedCAMEL_SubscriptionInfo_ss_CSI      = 6,
    RequestedCAMEL_SubscriptionInfo_m_CSI       = 7,
    RequestedCAMEL_SubscriptionInfo_d_csi       = 8
} RequestedCAMEL_SubscriptionInfo_e;
#endif /* _RequestedCAMEL_SubscriptionInfo_H_ */

namespace smsc {
namespace inman {
namespace comp {
namespace atih {

typedef struct {
    unsigned      serviceKey;
    TonNpiAddress scfAddress;
} MAPSCFinfo;

//MAP anyTimeInfohandling service
struct MAPATIH_OpCode {
    enum {
        anyTimeSubscriptionInterrogation = 62
    };
};

struct ERR_ATSI {
    enum {
        unknownSubscriber = 1,
        bearerServiceNotProvisioned = 10,
        teleserviceNotProvisioned = 11,
        callBarred = 13,
        illegalSSOperation = 16,
        ssNotAvailable = 18,
        dataMissing = 35,
        unexpectedDataValue = 36,
        atsi_NotAllowed = 60,
        informationNotAvailable = 62
    };
};

class ATSIArg : public Component {
public:
    ATSIArg(enum RequestedCAMEL_SubscriptionInfo
            csi2ask = RequestedCAMEL_SubscriptionInfo_o_CSI)
        : askCSI(csi2ask)
    { compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.ATSIArg"); }

    ~ATSIArg() { }

    //sets ISDN address of requesting point
    void setSCFaddress(const char * addr) throw(CustomException);
    void setSCFaddress(const TonNpiAddress& addr) { scfAdr = addr; }

    //sets subscriber identity: IMSI or MSISDN addr
    void setSubscriberId(const char * addr, bool imsi = true) throw(CustomException);
    //set subscriber identity as MSISDN addr
    void setSubscriberId(const TonNpiAddress& addr)
    { subscrAdr = addr; subscrImsi = false; }

    void setRequestedCSI(enum RequestedCAMEL_SubscriptionInfo csi2ask) { askCSI = csi2ask; }

    void encode(vector<unsigned char>& buf) throw(CustomException);

private:
    bool            subscrImsi;
    TonNpiAddress   subscrAdr;
    TonNpiAddress   scfAdr;
    enum RequestedCAMEL_SubscriptionInfo  askCSI;
    Logger*         compLogger;
};

class ATSIRes : public Component {
public:
    ATSIRes(): mask(0)
    { compLogger = smsc::logger::Logger::getInstance("smsc.inman.comp.ATSIRes"); }
    ~ATSIRes() { }

    bool isCSIpresent(enum RequestedCAMEL_SubscriptionInfo req_csi) const
    { return (mask & (1 << req_csi)) ? true : false; }

    //O-CSI only for now
    bool getSCFinfo(enum RequestedCAMEL_SubscriptionInfo req_csi, MAPSCFinfo * scf_dat) const;

    void decode(const vector<unsigned char>& buf) throw(CustomException);

private:
    unsigned    mask;
    Logger*     compLogger;
    MAPSCFinfo  o_csi;
};
}//namespace atih
}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_MAPATSI_COMPS_HPP__ */
