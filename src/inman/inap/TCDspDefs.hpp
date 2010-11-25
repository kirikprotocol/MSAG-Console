/* ************************************************************************* *
 * TCAP Dispatcher: SS7 stack interoperation parameters.
 * ************************************************************************* */
#ifndef __INMAN_TCAP_DISPATCHER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCAP_DISPATCHER_DEFS_HPP

#include <string>

#include "inman/inap/SS7UnitsDefs.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

#ifndef EIN_HD //EIN SS7 HR
struct SS7HRConnParms : public SS7ConnParms {
  static const uint16_t _DFLT_APPINSTANCE_ID = 0;

  SS7HRConnParms() : SS7ConnParms()
  {
    appInstId = _DFLT_APPINSTANCE_ID;
  }
};
typedef SS7HRConnParms  TCDsp_CFG;

#else  /* EIN_HD */

struct SS7HDConnParms : public SS7ConnParms {
  static const uint16_t _DFLT_APPINSTANCE_ID = 1;
  static const uint8_t  _MAX_CPMGR_INSTANCE_ID = 255;

  std::string   nmLayout;       //name of this SS7 units layout
  std::string   rcpMgrAdr;      //CSV list of remote CommonParts Managers host:port
  uint8_t       rcpMgrInstId;   //remote CommonParts Manager instanceId, [0..255],
                                //by default: 0

  SS7HDConnParms() : SS7ConnParms(), rcpMgrInstId(0)
  {
    appInstId = _DFLT_APPINSTANCE_ID;
  }
};
typedef SS7HDConnParms  TCDsp_CFG;

#endif /* EIN_HD */

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCAP_DISPATCHER_DEFS_HPP */

