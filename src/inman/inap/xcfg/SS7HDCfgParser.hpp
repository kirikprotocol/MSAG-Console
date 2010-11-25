#ifdef EIN_HD
/* ************************************************************************** *
 * EIN SS7 HD stack interaction parameters XML configuration parser.
 * ************************************************************************** */
#ifndef __INMAN_SS7HD_CFG_PARSER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_SS7HD_CFG_PARSER_HPP

#include "logger/Logger.h"
#include "util/config/XCFView.hpp"

#include "inman/inap/TCDspDefs.hpp"

namespace smsc  {
namespace inman {
namespace inap {

using smsc::logger::Logger;
using smsc::util::config::Config;
using smsc::util::config::XConfigView;
using smsc::util::config::ConfigException;

class SS7HDCfgParser {
private:
  Logger *        logger;
  const char *    nmSec;

protected:
  void readUnits(XConfigView & xcfg_sec, SS7HDConnParms & st_cfg, const char * nm_subs)
    throw(ConfigException);

  void readLayout(XConfigView & root_sec, SS7HDConnParms & st_cfg)
    throw(ConfigException);

public:
  explicit SS7HDCfgParser(Logger * use_log, const char * abs_nm_sec = "SS7_HD")
    : logger(use_log), nmSec(abs_nm_sec)
  { }
  ~SS7HDCfgParser()
  { }

  const char * nmCfgSection(void) const { return nmSec; }

  void readConfig(Config & root_sec, SS7HDConnParms & st_cfg) throw(ConfigException);
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_SS7HD_CFG_PARSER_HPP */
#endif /* EIN_HD */

