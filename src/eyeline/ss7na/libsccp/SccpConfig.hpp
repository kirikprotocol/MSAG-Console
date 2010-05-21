/* ************************************************************************** *
 * libSUA configuration data definition.
 * ************************************************************************** */
#ifndef __EYELINE_SS7NA_LIBSCCP_CFG_HPP
#ident "@(#)$Id$"
#define __EYELINE_SS7NA_LIBSCCP_CFG_HPP

#include <inttypes.h>
#include <netinet/in.h>
#include <string>
#include <vector>

namespace eyeline {
namespace ss7na {
namespace libsccp {

struct SccpConfig {
  enum TrafficMode_e {
    trfLOADSHARE = 0, trfOVERRIDE
  };
  struct LinkId {
    in_port_t   _port;
    std::string _host;
    std::string _name;

    LinkId() : _port(0)
    { }
    LinkId(const char * use_name, const char * use_host, in_port_t use_port)
      : _port(use_port)
    {
      if (use_host)
        _host = use_host;
      if (use_name)
        _name = use_name;
    }
  };

  typedef std::vector<LinkId>   LinksArray;

  std::string     _appId; //no longer than 255 chars
  TrafficMode_e   _trafficMode;
  LinksArray      _links;

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

