/* ************************************************************************** *
 * USS Request processing configuration.
 * ************************************************************************** */
#ifndef __SMSC_USSMAN_USSREQUEST_CFG_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_USSREQUEST_CFG_HPP

#include "inman/inap/TCUsrDefs.hpp"
#include "inman/inap/HDSSnSession.hpp"

namespace smsc  {
namespace ussman {

struct USSRequestCfg {
  smsc::inman::inap::TCAPUsr_CFG  _tcUsr;
  smsc::inman::inap::SSNSession * _ssnSess;

  USSRequestCfg() : _ssnSess(0)
  { }
};

struct USSConnectCfg {
  bool            _denyDupRequest;
  uint32_t        _maxRequests;
  uint16_t        _maxThreads;
  USSRequestCfg   _wrkCfg;  //request processor config

  USSConnectCfg() : _denyDupRequest(false), _maxRequests(0)
  { }
};

} //ussman
} //smsc

#endif /* __SMSC_USSMAN_USSREQUEST_CFG_HPP */

