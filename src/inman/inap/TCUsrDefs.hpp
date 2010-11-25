/* ************************************************************************* *
 * TCAP User parameters customizing interoperation with TCAP Dispatcher
 * ************************************************************************* */
#ifndef __INMAN_TCAP_USER_CFG_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCAP_USER_CFG_HPP

#include <inttypes.h>

#include "util/TonNpiAddress.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

using smsc::util::TonNpiAddress;

struct TCAPUsr_CFG { //TCAP User configuration
  static const uint16_t _DFLT_TCAP_DIALOGS = 800;
  static const uint16_t _MAX_TCAP_DIALOGS = 65510;
  static const uint16_t _DFLT_ROS_TIMEOUT = 20;   //seconds
  static const uint16_t _MAX_ROS_TIMEOUT = 600;   //seconds

  static const unsigned  _strSZ =
    (2*3 + 3 + TonNpiAddress::_strSZ + 1);  //"%s:%u{%u}"

  typedef smsc::core::buffers::FixedLengthString<_strSZ>
    StringForm_t;

  uint8_t         ownSsn;    //local SSN
  uint8_t         fakeSsn;   //SSN to substitute in TCAP dialog org addr,
                              //0 - means not used
  uint16_t        maxDlgId;   //maximum number of simultaneous TC dialogs
  uint16_t        rosTimeout; //optional timeout for ROS operations carried by TCAP
  TonNpiAddress   ownAddr;   //local ISDN address

  TCAPUsr_CFG() : ownSsn(0), fakeSsn(0), maxDlgId(0), rosTimeout(0)
  { }

  int toString(char* buf, unsigned buflen = TCAPUsr_CFG::_strSZ) const
  {
    int n = 0;
    if (fakeSsn)
      n = snprintf(buf, buflen - 1, "%s:%u{%u}", ownAddr.toString().c_str(),
                  (unsigned)ownSsn, (unsigned)fakeSsn);
    else
      n = snprintf(buf, buflen - 1, "%s:%u", ownAddr.toString().c_str(),
                   (unsigned)ownSsn);
    return n;
  }

  StringForm_t toString(void) const
  {
    StringForm_t  rval;
    toString(rval.str);
    return rval;
  }
};
typedef TCAPUsr_CFG::StringForm_t TCAPUsrCfgString_t;

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCAP_USER_CFG_HPP */

