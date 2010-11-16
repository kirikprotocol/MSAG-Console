#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <string.h>
#include "inman/comp/CSIDefs.hpp"

namespace smsc {
namespace inman {
namespace comp {

const char * UnifiedCSI::nmCSI(CSId_e cs_id)
{
  switch (cs_id) {
  case csi_O_BC:    return "O_CSI";
  case csi_T_BC:    return "T_CSI";
  case csi_VT:      return "VT_CSI";
  case csi_TIF:     return "TIF_CSI";
  case csi_GPRS:    return "GPRS_CSI";
  case csi_MO_SM:   return "MO_SM_CSI";
  case csi_SS:      return "SS_CSI";
  case csi_M:       return "M_CSI";
  case csi_D:       return "D_CSI";
  //
  case csi_MT_SM:   return "MT_SM_CSI";
  case csi_MG:      return "MG_CSI";
  case csi_O_IM:    return "O_IM_CSI";
  case csi_D_IM:    return "D_IM_CSI";
  case csi_VT_IM:   return "VT_IM_CSI";
  default:;
  }
  return "csi_UNDEFINED";
}

//TriggerDetectionPoints category names
static const char * _tdpNames[] = {
  "O-BC", "T-BC", "VT", "TIF", "GPRS"
  , "MO-SM", "SS", "M", "D", "MT-SM", "MG"
  , "O-IM", "D-IM", "VT-IM"
};

const char * UnifiedCSI::nmTDP(CSId_e cs_id)
{
  return ((unsigned)cs_id <= csi_VT_IM) ? _tdpNames[cs_id] : "tdp_UNDEFINED";
}

//
CSIUid_e UnifiedCSI::tdp2Id(const char * nm_tdp)
{
  unsigned i = 0;
  do {
    if (!strcmp(_tdpNames[i], nm_tdp))
      return static_cast<CSId_e>(i);
  } while ((++i) <= csi_VT_IM);

  return csi_UNDEFINED;
}


} //comp
} //inman
} //smsc


