/* ************************************************************************** *
 * USS Gateway (USSMan) configuration structure (matches the xml config).
 * ************************************************************************** */
#ifndef __SMSC_USSMAN_XCFG_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
# define __SMSC_USSMAN_XCFG_HPP

#include "inman/interaction/tcpserver/TcpServerCfg.hpp"
#include "inman/inap/TCDspDefs.hpp"
#include "inman/inap/TCUsrDefs.hpp"

namespace smsc  {
namespace ussman {

struct USSGateway_XCFG {
  bool      _denyDupRequest;
  uint32_t  _maxRequests;
  uint16_t  _maxThreads;
  /* - */
  smsc::inman::interaction::TcpServerCFG  _tcp;
  smsc::inman::inap::TCDsp_CFG            _ss7;
  smsc::inman::inap::TCAPUsr_CFG          _tcUsr;

  USSGateway_XCFG() : _denyDupRequest(false), _maxRequests(0), _maxThreads(0)
  { }
};

} //ussman
} //smsc

#endif /* __SMSC_USSMAN_XCFG_HPP */

