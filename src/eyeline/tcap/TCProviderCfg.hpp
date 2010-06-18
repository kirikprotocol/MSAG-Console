/* ************************************************************************** *
 * Transaction Capabilities Provider: configuration parameters.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVIDER_CFG_HPP
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVIDER_CFG_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/ss7na/libsccp/SccpConfig.hpp"
#include "eyeline/sccp/GlobalTitle.hpp"

namespace eyeline {
namespace tcap {

typedef std::vector<uint8_t>  SSNsArray;

struct TCProviderParms {
  static const TDlgTimeout _DFLT_DLG_TMO = 300;
  static const TDlgTimeout _DFLT_ROS_TMO = 60;

  sccp::GlobalTitle _ownGT;
  SSNsArray         _subSNs;
  uint32_t          _maxDlgNum;     //dflt = 0, must be set !!
  TDlgTimeout       _dfltDlgTmo;    //dflt = 300 sec
  TDlgTimeout       _dfltROSTmo;    //dflt = 60 sec

  unsigned          _iniMsgReaders; //dflt = 2
  unsigned          _maxMsgReaders; //dflt = 0, no limit

  TCProviderParms()
    : _maxDlgNum(0), _dfltDlgTmo(_DFLT_DLG_TMO), _dfltROSTmo(_DFLT_ROS_TMO)
    , _iniMsgReaders(2), _maxMsgReaders(0)
  { }
};

struct TCProviderCfg {
  TCProviderParms             _tcapCfg;
  ss7na::libsccp::SccpConfig  _sccpCfg;

  TCProviderCfg()
  { }
};

}}

#endif /* __EYELINE_TCAP_PROVIDER_CFG_HPP */

