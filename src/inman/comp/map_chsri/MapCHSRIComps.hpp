#ident "$Id$"
#ifndef __SMSC_INMAN_MAPCHSRI_COMPS_HPP__
#define __SMSC_INMAN_MAPCHSRI_COMPS_HPP__

#include "inman/common/adrutil.hpp"
#include "logger/Logger.h"
#include "inman/comp/compdefs.hpp"

using smsc::logger::Logger;
using smsc::inman::comp::Component;
using smsc::inman::comp::MAPComponent;
using smsc::inman::comp::MAPSCFinfo;

namespace smsc {
namespace inman {
namespace comp {
namespace chsri {

struct MAP_CH_SRI_OpCode {
    enum {
	sendRoutingInfo = 22
    };
};

struct ERR_CHSendRountigInfo {
    enum {
        systemFailure = 34,
        dataMissing = 35,
        unexpectedDataValue = 36,
        facilityNotSupported = 21,
        or_NotAllowed = 48,
        unknownSubscriber = 1,
        numberChanged = 44,
        bearerServiceNotProvisioned = 10,
        teleserviceNotProvisioned = 11,
        absentSubscriber = 27,
        busySubscriber = 45,
        noSubscriberReply = 46,
        callBarred = 13,
        cug_Reject = 15,
        forwardingViolation = 14
    };
};


class CHSendRoutingInfoArg : public Component {
public:
    CHSendRoutingInfoArg();
    ~CHSendRoutingInfoArg() { }

    //sets ISDN address of requesting point
    void setGMSCorSCFaddress(const char * addr) throw(CustomException);
    void setGMSCorSCFaddress(const TonNpiAddress& addr) { scfAdr = addr; }

    //sets subscriber MSISDN addr
    void setSubscrMSISDN(const char * msisdn) throw(CustomException);
    void setSubscrMSISDN(const TonNpiAddress& addr) { subscrAdr = addr; }

    void encode(vector<unsigned char>& buf) throw(CustomException);

private:
    TonNpiAddress      	scfAdr;     //requesting GMSC or SCF address
    TonNpiAddress      	subscrAdr;  // MS ISDN address, '\0' or "0" for absence
    Logger*		compLogger;
};


class CHSendRoutingInfoRes : public MAPComponent {
public:
    CHSendRoutingInfoRes();
    ~CHSendRoutingInfoRes() {}

    int getSCFinfo(MAPSCFinfo * scf_dat) const;
    int getIMSI(char *imsi) const;

    void decode(const vector<unsigned char>& buf) throw(CustomException);
    void mergeSegment(Component * segm) throw(CustomException);

    bool hasIMSI(void) const { return mask.imsi; }
    bool hasOCSI(void) const { return mask.o_csi; }

private:
    struct {
        unsigned int imsi  :1 ; 
        unsigned int o_csi :1 ; 
    } mask;
    MAPSCFinfo  o_csi;
    char          o_imsi[MAP_MAX_IMSI_AddressValueLength + 1];
    Logger*	  compLogger;
};

} //chsri
} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPCHSRI_COMPS_HPP__ */

