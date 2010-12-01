#ifndef EIN_HD //EIN SS7 HR
/* ************************************************************************** *
 * EIN SS7 HR stack interoperation parameters XML configuration parser.
 * ************************************************************************** */
#ifndef __INMAN_SS7HR_CFG_PARSER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_SS7HR_CFG_PARSER_HPP

#include "logger/Logger.h"
#include "util/config/Config.h"
#include "inman/inap/TCDspDefs.hpp"

namespace smsc  {
namespace inman {
namespace inap {

using smsc::logger::Logger;
using smsc::util::config::Config;
using smsc::util::config::ConfigException;

class SS7HRCfgParser {
private:
  Logger *        logger;
  const char *    nmSec;

public:
  explicit SS7HRCfgParser(Logger * use_log, const char * abs_nm_sec = "SS7")
    : logger(use_log), nmSec(abs_nm_sec)
  { }
  ~SS7HRCfgParser()
  { }

  const char * nmCfgSection(void) const { return nmSec; }

  void readConfig(const Config & root_sec, SS7HRConnParms & st_cfg) throw(ConfigException);
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_SS7HR_CFG_PARSER_HPP */
#endif /* EIN_HD */

