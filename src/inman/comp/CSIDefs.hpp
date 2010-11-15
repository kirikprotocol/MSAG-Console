/* ************************************************************************* *
 * CAMEL Subsciption Info ids definition.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_CSI_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_CSI_DEFS_HPP

/* NOTE: while linking the below enums are taken from generated *
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

namespace smsc {
namespace inman {
namespace comp {

//UnifiedCAMELSubscriptionInfo: concatenation of 
//  RequestedCAMEL_SubscriptionInfo_e and
//  AdditionalRequestedCAMEL_SubscriptionInfo_e
class UnifiedCSI {
protected:
  unsigned _value;

public:
  typedef unsigned value_type;

  static const unsigned _strSZ_CSI = (unsigned)sizeof("csi_UNDEFINED");
  static const unsigned _strSZ_TDP = (unsigned)sizeof("tdp_UNDEFINED");
  static const unsigned _xCSI_Idx = 9;

  enum CSId_e { 
    //basic CSI follows
      csi_O_BC  = 0
    , csi_T_BC  = 1
    , csi_VT    = 2
    , csi_TIF   = 3
    , csi_GPRS  = 4
    , csi_MO_SM = 5
    , csi_SS    = 6
    , csi_M     = 7
    , csi_D     = 8
    //additional CSI follows
    , csi_MT_SM = (_xCSI_Idx + 0)
    , csi_MG    = (_xCSI_Idx + 1)
    , csi_O_IM  = (_xCSI_Idx + 2)
    , csi_D_IM  = (_xCSI_Idx + 3)
    , csi_VT_IM = (_xCSI_Idx + 4)
    // enum is extensible
    , csi_UNDEFINED = (unsigned)(-1)
  };

  explicit UnifiedCSI(CSId_e csi_id = csi_UNDEFINED)
    : _value(csi_id)
  { }
  explicit UnifiedCSI(RequestedCAMEL_SubscriptionInfo_e cs_id)
    : _value(cs_id)
  { }
  explicit UnifiedCSI(AdditionalRequestedCAMEL_SubscriptionInfo_e cs_id)
    : _value(_xCSI_Idx + cs_id)
  { }
  ~UnifiedCSI()
  { }

  static CSId_e get(unsigned ui_val)
  {
    return (ui_val > csi_VT_IM) ? csi_UNDEFINED : static_cast<CSId_e>(ui_val);
  }

  CSId_e get(void) const { return static_cast<CSId_e>(_value); }

  void set(CSId_e cs_id) { _value = cs_id; }
  
  //return name of given CSI
  static const char * nmCSI(CSId_e cs_id);
  //return name of TDPs category associated with given CSI
  static const char * nmTDP(CSId_e cs_id);
  //
  static CSId_e tdp2Id(const char * nm_tdp);
  //
  static bool isBasicCSI(value_type cs_id)      { return cs_id < _xCSI_Idx; }
  static bool isAdditionalCSI(value_type cs_id) { return cs_id >= _xCSI_Idx; }
  
  bool isBasicCSI()       const { return isBasicCSI(_value); }
  bool isAdditionalCSI()  const { return isAdditionalCSI(_value); }
  //
  //return name of this CSI
  const char * nmCSI(void) const { return nmCSI(get()); }
  //return name of TDPs category associated with this CSI
  const char * nmTDP(void)  const { return nmTDP(get()); }

  //NOTE: it's a caller responsibility to determine type of CSI (basic or additional)
  //prior to calling these methods
  RequestedCAMEL_SubscriptionInfo_e getBasicCSI() const
  {
    return static_cast<RequestedCAMEL_SubscriptionInfo_e>(_value);
  }
  AdditionalRequestedCAMEL_SubscriptionInfo_e getAdditionalCSI() const
  {
    return static_cast<AdditionalRequestedCAMEL_SubscriptionInfo_e>(_value - _xCSI_Idx);
  }

  void setBasicCSI(RequestedCAMEL_SubscriptionInfo_e cs_id)
  {
    _value = cs_id;
  }
  void setAdditionalCSI(AdditionalRequestedCAMEL_SubscriptionInfo_e cs_id)
  {
    _value = _xCSI_Idx + cs_id;
  }
};

typedef UnifiedCSI::CSId_e CSIUid_e;

}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_CSI_DEFS_HPP */
