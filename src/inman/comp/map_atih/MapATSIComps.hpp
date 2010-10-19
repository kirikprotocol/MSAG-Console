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

#ifndef	_AdditionalRequestedCAMEL_SubscriptionInfo_H_
/* this is the clone of AdditionalRequestedCAMEL-SubscriptionInfo.h::AdditionalRequestedCAMEL_SubscriptionInfo_e */
typedef enum AdditionalRequestedCAMEL_SubscriptionInfo {
	AdditionalRequestedCAMEL_SubscriptionInfo_mt_sms_CSI	= 0,
	AdditionalRequestedCAMEL_SubscriptionInfo_mg_csi	= 1,
	AdditionalRequestedCAMEL_SubscriptionInfo_o_IM_CSI	= 2,
	AdditionalRequestedCAMEL_SubscriptionInfo_d_IM_CSI	= 3,
	AdditionalRequestedCAMEL_SubscriptionInfo_vt_IM_CSI	= 4
	/*
	 * Enumeration is extensible
	 */
} AdditionalRequestedCAMEL_SubscriptionInfo_e;
#endif /* _AdditionalRequestedCAMEL_SubscriptionInfo_H_ */

#ifndef	_BasicServiceCode_H_
typedef enum BasicServiceCode_PR {
	BasicServiceCode_PR_NOTHING,	/* No components present */
	BasicServiceCode_PR_bearerService,
	BasicServiceCode_PR_teleservice
} BasicServiceCode_PR;
#endif /* _BasicServiceCode_H_ */

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


struct RequestedSubscriptionFlags_s {
  uint8_t ss   : 1;   //Supplementary Services info
  uint8_t odb  : 1;   //Operator Determined Barring
  uint8_t csi  : 1;   //CAMEL Subscription info (see RequestedCAMEL_SubscriptionInfo_e)
  uint8_t xcsi : 1;   //Additional CAMEL Subscription info (see AdditionalRequestedCAMEL_SubscriptionInfo_e)
  uint8_t vlr  : 1;   //Supported Camel Phases in VLR
  uint8_t sgsn : 1;   //Supported Camel Phases in SGSN
//  uint8_t bslist : 1; //BasicServices list per MSISDN (intriduced in MAP v7.15.0)
};

struct RequestedSS_ForBS {
  uint8_t               ssCode;
  BasicServiceCode_PR   bsCodeKind;
  union {
    uint8_t             bearer;
    uint8_t             tele;
  } bsCode;

  RequestedSS_ForBS() : ssCode(0), bsCodeKind(BasicServiceCode_PR_NOTHING) 
  { }
  ~RequestedSS_ForBS()
  { }

  void clear(void)
  {
    ssCode = bsCode.tele = 0;
    bsCodeKind = BasicServiceCode_PR_NOTHING; 
  }

  void setRequestedSSInfo(uint8_t ss_code)
  {
    ssCode = ss_code;
    bsCodeKind = BasicServiceCode_PR_NOTHING;
  }
  void setRequestedSSInfoforBearer(uint8_t ss_code, uint8_t bs_code)
  {
    ssCode = ss_code; bsCode.bearer = bs_code;
    bsCodeKind = BasicServiceCode_PR_bearerService;
  }
  void setRequestedSSInfoforTele(uint8_t ss_code, uint8_t ts_code)
  {
    ssCode = ss_code; bsCode.tele = ts_code; 
    bsCodeKind = BasicServiceCode_PR_teleservice;
  }
};

class RequestedSubscription {
protected:
  RequestedCAMEL_SubscriptionInfo_e           idCSI;
  AdditionalRequestedCAMEL_SubscriptionInfo_e idXCSI;
  RequestedSS_ForBS                           idSS;

  union {
    uint8_t                       ui;
    RequestedSubscriptionFlags_s  s;
  }   flags;

public:
  RequestedSubscription()
  {
    flags.ui = 0;
  }
  explicit RequestedSubscription(RequestedCAMEL_SubscriptionInfo_e req_csi)
    : idCSI(req_csi)
  {
    flags.ui = 0; flags.s.csi = 1;
  }
  ~RequestedSubscription()
  { }

  void clear(void) { flags.ui = 0; idSS.clear(); }

  uint8_t getFlagsValue(void) const { return flags.ui; }
  const RequestedSubscriptionFlags_s & getFlags(void) const { return flags.s; }

  RequestedCAMEL_SubscriptionInfo_e getRequestedCSI(void) const { return idCSI; }
  AdditionalRequestedCAMEL_SubscriptionInfo_e getRequestedXCSI(void) const { return idXCSI; }

  const RequestedSS_ForBS & getRequestedSSInfo(void) const { return idSS; }

  void setRequestedCSI(RequestedCAMEL_SubscriptionInfo_e csi2ask)
  {
    idCSI = csi2ask; flags.s.csi = 1;
  }
  void unsetRequestedCSI(void)
  {
    flags.s.csi = 0;
  }
  void setRequestedXCSI(AdditionalRequestedCAMEL_SubscriptionInfo_e xcsi2ask)
  {
    idXCSI = xcsi2ask; flags.s.xcsi = 1;
  }
  void unsetRequestedXCSI(void)
  {
    flags.s.xcsi = 0;
  }

  void setRequestedODB(bool is_set = true) { flags.s.odb = is_set ? 1 : 0; }
  void setRequestedVLRPhases(bool is_set = true) { flags.s.vlr = is_set ? 1 : 0; }
  void setRequestedSGSNPhases(bool is_set = true) { flags.s.sgsn = is_set ? 1 : 0; }


//  void setRequestedBSList(void) { flags.s.bslist = 1; }

  void setRequestedSSInfo(uint8_t ss_code)
  {
    idSS.setRequestedSSInfo(ss_code); flags.s.ss = 1;
  }
  void setRequestedSSInfoforBearer(uint8_t ss_code, uint8_t bs_code)
  {
    idSS.setRequestedSSInfoforBearer(ss_code, bs_code); flags.s.ss = 1;
  }
  void setRequestedSSInfoforTele(uint8_t ss_code, uint8_t ts_code)
  {
    idSS.setRequestedSSInfoforTele(ss_code, ts_code); flags.s.ss = 1;
  }
  void unsetRequestedSSInfo(void)
  {
    flags.s.ss = 0; idSS.clear();
  }
};


class ATSIArg : public Component, public RequestedSubscription {
public:
    ATSIArg(Logger * use_log = NULL) : RequestedSubscription()
      , subscrImsi(false)
      , compLogger(use_log ? use_log : Logger::getInstance("smsc.inman.comp.ATSIArg"))
    { }
    ATSIArg(const RequestedSubscription & req_info, Logger * use_log = NULL)
      : RequestedSubscription(req_info)
      , subscrImsi(false)
      , compLogger(use_log ? use_log : Logger::getInstance("smsc.inman.comp.ATSIArg"))
    { }

    ~ATSIArg()
    { }

    //sets ISDN address of requesting point
    void setSCFaddress(const char * addr) throw(CustomException);
    void setSCFaddress(const TonNpiAddress& addr) { scfAdr = addr; }

    //sets subscriber identity: IMSI or MSISDN addr
    void setSubscriberId(const char * addr, bool imsi = true) throw(CustomException);
    //set subscriber identity as MSISDN addr
    void setSubscriberId(const TonNpiAddress& addr)
    {
      subscrAdr = addr; subscrImsi = false;
    }

    void encode(std::vector<unsigned char>& buf) const throw(CustomException);

private:
  bool                  subscrImsi; //subscriber is identified by IMSI, not by MSISDN
  TonNpiAddress         subscrAdr;
  TonNpiAddress         scfAdr;
  Logger*               compLogger;
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
