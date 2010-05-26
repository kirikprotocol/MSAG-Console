/* ************************************************************************** *
 * libSUA XML configurations parser.
 * ************************************************************************** */
#ifndef __EYELINE_SS7NA_LIBSSCP_CFG_PARSER_HPP
#ident "@(#)$Id$"
#define __EYELINE_SS7NA_LIBSSCP_CFG_PARSER_HPP

#include "eyeline/ss7na/libsccp/SccpConfig.hpp"
#include "util/config/XCFView.hpp"

namespace eyeline  {
namespace ss7na {
namespace libsccp {

using smsc::util::config::XConfigView;
using smsc::util::config::ConfigException;

class LibSccpCfgReader {
private:
  const char *    _nmSec;

protected:
  void readLinkParms(XConfigView * outer_sec, const char * nm_sec,
                     SccpConfig::SCSPLinksArray & links_arr)
       throw(ConfigException);

  //return false if no links is defined
  bool readLinks(XConfigView * outer_sec, SccpConfig::SCSPLinksArray & links_arr)
       throw(ConfigException);

public:
  LibSccpCfgReader(const char * abs_nm_sec = "sccp_provider")
    : _nmSec(abs_nm_sec)
  { }
  ~LibSccpCfgReader()
  { }

  const char * nmCfgSection(void) const { return _nmSec; }

  void readConfig(XConfigView * root_sec, SccpConfig & st_cfg)
      throw(ConfigException);
};

} //libsccp
} //ss7na
} //eyeline
#endif /* __EYELINE_SS7NA_LIBSSCP_CFG_PARSER_HPP */

