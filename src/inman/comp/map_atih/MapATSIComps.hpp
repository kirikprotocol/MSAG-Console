/* ************************************************************************* *
 * MAP ATIH AnyTimeSubscriptionInterrogation Components definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPATSI_COMPS_HPP__
#ident "@(#)$Id$"
#define __SMSC_INMAN_MAPATSI_COMPS_HPP__

#include "logger/Logger.h"

#include "inman/GsmSCFInfo.hpp"
#include "inman/common/adrutil.hpp"
#include "inman/comp/compdefs.hpp"
#include "inman/comp/MapOpErrors.hpp"

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

using smsc::logger::Logger;

using smsc::inman::comp::Component;
using smsc::inman::comp::MAPOpErrorId;

//MAP anyTimeInfohandling service
struct MAPATIH_OpCode {
    enum {
        anyTimeSubscriptionInterrogation = 62
    };
};

struct ERR_ATSI {
    enum {
        unknownSubscriber           = MAPOpErrorId::unknownSubscriber,
        bearerServiceNotProvisioned = MAPOpErrorId::bearerServiceNotProvisioned,
        teleserviceNotProvisioned   = MAPOpErrorId::teleserviceNotProvisioned,
        callBarred                  = MAPOpErrorId::callBarred,
        illegalSSOperation          = MAPOpErrorId::illegalSS_Operation,
        ssNotAvailable              = MAPOpErrorId::ss_NotAvailable,
        dataMissing                 = MAPOpErrorId::dataMissing,
        unexpectedDataValue         = MAPOpErrorId::unexpectedDataValue,
        atsi_NotAllowed             = MAPOpErrorId::atsi_NotAllowed,
        informationNotAvailable     = MAPOpErrorId::informationNotAvailable
    };
};

class ATSIArg : public Component {
public:
    ATSIArg(Logger * use_log = NULL)
        : askCSI(RequestedCAMEL_SubscriptionInfo_o_CSI)
        , compLogger(use_log ? use_log : Logger::getInstance("smsc.inman.comp.ATSIArg"))
    { }
    ~ATSIArg()
    { }

    //sets ISDN address of requesting point
    void setSCFaddress(const char * addr) throw(CustomException);
    inline void setSCFaddress(const TonNpiAddress& addr) { scfAdr = addr; }

    //sets subscriber identity: IMSI or MSISDN addr
    void setSubscriberId(const char * addr, bool imsi = true) throw(CustomException);
    //set subscriber identity as MSISDN addr
    inline void setSubscriberId(const TonNpiAddress& addr)
    { subscrAdr = addr; subscrImsi = false; }

    inline void setRequestedCSI(enum RequestedCAMEL_SubscriptionInfo csi2ask)
    { askCSI = csi2ask; }

    void encode(std::vector<unsigned char>& buf) const throw(CustomException);

private:
    bool            subscrImsi;
    TonNpiAddress   subscrAdr;
    TonNpiAddress   scfAdr;
    enum RequestedCAMEL_SubscriptionInfo  askCSI;
    Logger*         compLogger;
};

class ATSIRes : public Component {
public:
    ATSIRes(Logger * use_log = NULL)
        : mask(0), compLogger(use_log ? use_log : 
                                Logger::getInstance("smsc.inman.comp.ATSIRes"))
    { }
    ~ATSIRes()
    { }

    bool isCSIpresent(enum RequestedCAMEL_SubscriptionInfo req_csi) const
    { return (mask & (1 << req_csi)) ? true : false; }

    //O-CSI only for now
    bool getSCFinfo(enum RequestedCAMEL_SubscriptionInfo req_csi, GsmSCFinfo * scf_dat) const;

    void decode(const std::vector<unsigned char>& buf) throw(CustomException);

private:
    unsigned    mask;
    Logger*     compLogger;
    GsmSCFinfo  o_csi;
};
}//namespace atih
}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_MAPATSI_COMPS_HPP__ */
