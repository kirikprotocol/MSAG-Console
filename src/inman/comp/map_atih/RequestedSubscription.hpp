/* ************************************************************************* *
 * Helpers for identification of requested CSIs in ATIH Operations.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_REQUESTED_CSI_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_REQUESTED_CSI_HPP

#include "inman/comp/CSIDefs.hpp"

/* GVR NOTE: while linking the below enums are taken from generated *
 * asn1/c codec, so they should not have namespace prefix.          */

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

using smsc::inman::comp::UnifiedCSI;
using smsc::inman::comp::CSIUid_e;

struct RequestedSubscriptionFlags_s {
  uint8_t ss   : 1;   //Supplementary Services info
  uint8_t odb  : 1;   //Operator Determined Barring
  uint8_t csi  : 1;   //CAMEL Subscription info (see RequestedCAMEL_SubscriptionInfo_e)
  uint8_t xcsi : 1;   //Additional CAMEL Subscription info (see AdditionalRequestedCAMEL_SubscriptionInfo_e)
  uint8_t vlr  : 1;   //Supported Camel Phases in VLR
  uint8_t sgsn : 1;   //Supported Camel Phases in SGSN
//  uint8_t bslist : 1; //BasicServices list per MSISDN (introduced in MAP v7.15.0)
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
  UnifiedCSI          idCSI;
  RequestedSS_ForBS   idSS;

  union {
    uint8_t                       ui;
    RequestedSubscriptionFlags_s  s;
  }   flags;

public:
  RequestedSubscription() : idCSI(UnifiedCSI::csi_UNDEFINED)
  {
    flags.ui = 0;
  }
  explicit RequestedSubscription(UnifiedCSI::CSId_e req_csi) : idCSI(req_csi)
  {
    flags.ui = 0;
    if (idCSI.isBasicCSI())
      flags.s.csi = 1;
    else
      flags.s.xcsi = 1;
  }
  ~RequestedSubscription()
  { }

  void clear(void) { flags.ui = 0; idSS.clear(); }

  uint8_t getFlagsValue(void) const { return flags.ui; }
  const RequestedSubscriptionFlags_s & getFlags(void) const { return flags.s; }

  bool hasCSI(void) const { return (flags.s.csi || flags.s.xcsi); }

  CSIUid_e getCSI(void) const
  {
    return hasCSI() ? idCSI.get() : UnifiedCSI::csi_UNDEFINED;
  }

  const char * nmCSI(void) const { return idCSI.nmCSI(); }

  void setCSI(CSIUid_e cs_uid)
  {
    idCSI.set(cs_uid);
    if (idCSI.isBasicCSI())
      flags.s.csi = 1;
    else
      flags.s.xcsi = 1;
  }
  void unsetCSI(CSIUid_e cs_uid)
  {
    if (idCSI.isBasicCSI())
      unsetBasicCSI();
    else
      unsetAdditionalCSI();
  }

  RequestedCAMEL_SubscriptionInfo_e getBasicCSI(void) const
  {
    return idCSI.getBasicCSI();
  }
  AdditionalRequestedCAMEL_SubscriptionInfo_e getAdditionalCSI(void) const
  {
    return idCSI.getAdditionalCSI();
  }

  const RequestedSS_ForBS & getRequestedSSInfo(void) const { return idSS; }

  void setBasicCSI(RequestedCAMEL_SubscriptionInfo_e csi_id)
  {
    idCSI.setBasicCSI(csi_id); flags.s.csi = 1;
  }
  void unsetBasicCSI(void)
  {
    idCSI.set(UnifiedCSI::csi_UNDEFINED);
    flags.s.csi = 0; 
  }
  void setAdditionalCSI(AdditionalRequestedCAMEL_SubscriptionInfo_e xcsi_id)
  {
    idCSI.setAdditionalCSI(xcsi_id); flags.s.xcsi = 1;
  }
  void unsetAdditionalCSI(void)
  {
    idCSI.set(UnifiedCSI::csi_UNDEFINED);
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

}//namespace atih
}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_REQUESTED_CSI_HPP */

