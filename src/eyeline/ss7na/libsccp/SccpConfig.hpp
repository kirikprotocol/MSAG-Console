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

//SCCP Service Provider link ident
struct SCSPLinkId {
  std::string _name;
  std::string _host;
  in_port_t   _port;

  SCSPLinkId() : _port(0)
  { }
  SCSPLinkId(const char * use_name, const char * use_host, in_port_t use_port)
    : _port(use_port)
  {
    if (use_host)
      _host = use_host;
    if (use_name)
      _name = use_name;
  }
};

//SCCP Service Provider link state
struct SCSPLinkState {
  enum Status_e { linkNOT_CONNECTED = 0, linkCONNECTED, linkBINDED };

  Status_e          _connStatus;
  sccp::SCCPAddress _sccpAddr;

  SCSPLinkState() : _connStatus(linkNOT_CONNECTED)
  { }
};

//SCCP Service Provider link
struct SCSPLink {
  SCSPLinkId    _id;
  SCSPLinkState _state;

  SCSPLink()
  { }
  SCSPLink(const char * use_name, const char * use_host, in_port_t use_port)
    : _id(use_name, use_host, use_port)
  { }
};


typedef SCSPLinkState::Status_e SCSPLinkStatus_e;

struct SccpConfig {
  enum TrafficMode_e {
    trfLOADSHARE = 0, trfOVERRIDE
  };

  typedef std::vector<SCSPLink> SCSPLinksArray;

  std::string     _appId; //no longer than 255 chars
  TrafficMode_e   _trafficMode;
  SCSPLinksArray  _links;

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

