/* ************************************************************************** *
 * Configurable Services Host config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_TCPSERVER_CONFIG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCPSERVER_CONFIG_PARSING__

#include "inman/interaction/tcpserver/TcpServerCfg.hpp"

#include "inman/services/ICSCfgReader.hpp"

namespace smsc {
namespace inman {
namespace tcpsrv {

using smsc::inman::XMFConfig;
using smsc::inman::interaction::TcpServerCFG;

class ICSTcpSrvCfgReader : public smsc::inman::ICSrvCfgReaderAC_T<TcpServerCFG> {
protected:
  static const unsigned int _DFLT_CLIENT_CONNS = 3;

  // --------------------------------------------------
  // ICSrvCfgReaderAC interface methods implementation
  // --------------------------------------------------
  //Parses XML configuration entry section, updates dependencies.
  //Returns status of config parsing
  virtual CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException);

public:
  ICSTcpSrvCfgReader(XMFConfig & xmf_cfg, Logger * use_log, const char * ics_sec = NULL)
      : smsc::inman::ICSrvCfgReaderAC_T<TcpServerCFG>(xmf_cfg, use_log, ics_sec ? ics_sec : "Host")
  { }
  ~ICSTcpSrvCfgReader()
  { }
};


} //tcpsrv
} //inman
} //smsc
#endif /* __INMAN_TCPSERVER_CONFIG_PARSING__ */

