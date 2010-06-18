/* ************************************************************************** *
 * libSUA XML configurations parser.
 * ************************************************************************** */
#ifndef __EYELINE_TCPROVD_CFG_PARSER_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCPROVD_CFG_PARSER_HPP

#include "eyeline/tcap/TCProviderCfg.hpp"
#include "util/config/XCFView.hpp"

namespace eyeline  {
namespace tcap {

using smsc::util::config::Config;
using smsc::util::config::ConfigException;

class TCProvdCfgReader {
private:
  const char *    _nmSec;

public:
  TCProvdCfgReader(const char * abs_nm_sec = "tcap_provider")
    : _nmSec(abs_nm_sec)
  { }
  ~TCProvdCfgReader()
  { }

  const char * nmCfgSection(void) const { return _nmSec; }

  void readConfig(Config & use_xcfg, TCProviderCfg & st_cfg)
      throw(ConfigException);
};


} //tcap
} //eyeline
#endif /* __EYELINE_TCPROVD_CFG_PARSER_HPP */

