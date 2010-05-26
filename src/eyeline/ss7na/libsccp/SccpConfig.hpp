/* ************************************************************************** *
 * libSCCP configuration data definition.
 * ************************************************************************** */
#ifndef __EYELINE_SS7NA_LIBSCCP_CFG_HPP
#ident "@(#)$Id$"
#define __EYELINE_SS7NA_LIBSCCP_CFG_HPP

#include <inttypes.h>
#include <netinet/in.h>
#include <string>
#include <vector>

#include "eyeline/sccp/SCCPAddress.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

//Id & parameteres of link to SCCP Provider
struct LinkId {
  enum State_e { linkNOT_CONNECTED, linkCONNECTED, linkBINDED };
  std::string _name;
  std::string _host;
  in_port_t   _port;
  State_e     _connState;
  sccp::SCCPAddress _sccpAddr;

  LinkId() : _port(0), _connState(linkNOT_CONNECTED)
  { }
  LinkId(const char * use_name, const char * use_host, in_port_t use_port)
    : _port(use_port), _connState(linkNOT_CONNECTED)
  {
    if (use_host)
      _host = use_host;
    if (use_name)
      _name = use_name;
  }
};

typedef LinkId::State_e LinkState_e;

struct SccpConfig {
  enum TrafficMode_e {
    trfLOADSHARE = 0, trfOVERRIDE
  };

  typedef std::vector<LinkId> LinksArray;

  std::string     _appId; //no longer than 255 chars
  TrafficMode_e   _trafficMode;
  LinksArray      _links; //links to SCCP Providers

  SccpConfig() : _trafficMode(trfLOADSHARE)
  { }

  static const char * nmTrafficMode(TrafficMode_e use_val)
  {
    if (use_val == trfLOADSHARE)
      return "LOADSHARE";
    if (use_val == trfOVERRIDE)
      return "OVERRIDE";
    return "undefined";
  }

  const char * nmTrafficMode(void) const
  {
    return nmTrafficMode(_trafficMode);
  }
};

}}}

#endif /* __EYELINE_SS7NA_LIBSCCP_CFG_HPP */

