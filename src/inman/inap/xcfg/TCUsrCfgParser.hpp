/* ************************************************************************** *
 * TCAP User XML configuration parser.
 * ************************************************************************** */
#ifndef __INMAN_TCAP_USER_CFG_PARSER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCAP_USER_CFG_PARSER_HPP

#include "logger/Logger.h"
#include "util/config/Config.h"
#include "inman/inap/TCUsrDefs.hpp"

namespace smsc  {
namespace inman {
namespace inap {

using smsc::logger::Logger;
using smsc::util::config::Config;
using smsc::util::config::ConfigException;

class TCAPUsrCfgParser {
private:
  Logger *        logger;
  const char *    nmSec;

public:
  explicit TCAPUsrCfgParser(Logger * use_log, const char * abs_nm_sec = "TCAPUser")
    : logger(use_log), nmSec(abs_nm_sec)
  { }
  ~TCAPUsrCfgParser()
  { }

  const char * nmCfgSection(void) const { return nmSec; }

  void readConfig(Config & root_sec, TCAPUsr_CFG & st_cfg) throw(ConfigException);
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCAP_USER_CFG_PARSER_HPP */

