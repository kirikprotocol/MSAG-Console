#pragma ident "$Id$"
/* ************************************************************************* *
 * MAP CallHandling service SEND_ROUTING_INFO (v3) Components definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPCHSRI_COMPS_HPP__
#define __SMSC_INMAN_MAPCHSRI_COMPS_HPP__

#include "logger/Logger.h"
#include "inman/common/adrutil.hpp"
#include "inman/comp/compdefs.hpp"
#include "inman/comp/MapOpErrors.hpp"

using smsc::logger::Logger;
using smsc::inman::comp::Component;
using smsc::inman::comp::MAPComponent;
using smsc::inman::comp::MAPOpErrorId;

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
        systemFailure       = MAPOpErrorId::systemFailure,
        dataMissing         = MAPOpErrorId::dataMissing,
        unexpectedDataValue = MAPOpErrorId::unexpectedDataValue,
        facilityNotSupported = MAPOpErrorId::facilityNotSupported,
        or_NotAllowed       = MAPOpErrorId::or_NotAllowed,
        unknownSubscriber   = MAPOpErrorId::unknownSubscriber,
        numberChanged       = MAPOpErrorId::numberChanged,
        bearerServiceNotProvisioned = MAPOpErrorId::bearerServiceNotProvisioned,
        teleserviceNotProvisioned = MAPOpErrorId::teleserviceNotProvisioned,
        absentSubscriber    = MAPOpErrorId::absentSubscriber,
        busySubscriber      = MAPOpErrorId::busySubscriber,
        noSubscriberReply   = MAPOpErrorId::noSubscriberReply,
        callBarred          = MAPOpErrorId::callBarred,
        cug_Reject          = MAPOpErrorId::cug_Reject,
        forwardingViolation = MAPOpErrorId::forwardingViolation
    };
};


class CHSendRoutingInfoArg : public Component {
public:
    CHSendRoutingInfoArg(Logger * use_log = NULL)
        : compLogger(use_log ? use_log : 
                     Logger::getInstance("smsc.inman.comp.CHSRIArg"))
    { }
    ~CHSendRoutingInfoArg()
    { }

    //sets ISDN address of requesting point
    void setGMSCorSCFaddress(const char * addr) throw(CustomException);
    void setGMSCorSCFaddress(const TonNpiAddress& addr) { scfAdr = addr; }

    //sets subscriber MSISDN addr
    void setSubscrMSISDN(const char * msisdn) throw(CustomException);
    void setSubscrMSISDN(const TonNpiAddress& addr) { subscrAdr = addr; }

    void encode(std::vector<unsigned char>& buf) const throw(CustomException);

private:
    TonNpiAddress      	scfAdr;     //requesting GMSC or SCF address
    TonNpiAddress      	subscrAdr;  // MS ISDN address, '\0' or "0" for absence
    Logger*		compLogger;
};


class CHSendRoutingInfoRes : public MAPComponent {
public:
    CHSendRoutingInfoRes(Logger * use_log = NULL)
        : compLogger(use_log ? use_log : 
                 Logger::getInstance("smsc.inman.comp.CHSRIRes"))
    {
        mask.value = o_imsi[0] = 0;
    }
    ~CHSendRoutingInfoRes()
    {}

    unsigned short getSCFinfo(GsmSCFinfo * scf_dat) const;
    unsigned short getIMSI(char *imsi) const;
    unsigned short getVLRN(TonNpiAddress & vlr_n) const;


    void decode(const std::vector<unsigned char>& buf) throw(CustomException);
    void mergeSegment(Component * segm) throw(CustomException);

    inline bool hasIMSI(void) const { return mask.st.imsi; }
    inline bool hasOCSI(void) const { return mask.st.o_csi; }
    inline bool hasVLRN(void) const { return mask.st.n_vlr; }

private:
    union {
        unsigned short value;
        struct {
            unsigned short imsi  :1 ; 
            unsigned short o_csi :1 ;
            unsigned short n_vlr :1 ;
        } st;
    } mask;
    TonNpiAddress vlrNum;   //VLR number
    GsmSCFinfo    o_csi;
    char          o_imsi[MAP_MAX_IMSI_AddressValueLength + 1];
    Logger*	  compLogger;
};

} //chsri
} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPCHSRI_COMPS_HPP__ */

