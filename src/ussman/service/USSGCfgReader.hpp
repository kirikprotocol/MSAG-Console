/* ************************************************************************** *
 * USS Gateway (USSMan) XML config file parser.
 * ************************************************************************** */
#ifndef __SMSC_USSMAN_CFG_READER_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_CFG_READER_HPP__

#include "logger/Logger.h"

#include "ussman/service/USSGCfgDefs.hpp"

namespace smsc  {
namespace ussman {

using smsc::logger::Logger;

using smsc::util::config::Config;
using smsc::util::config::ConfigException;

class USSGCfgReader {
private:
  const char *    nmSrv; //name of USS Service
  Logger *        logger;
  //structure containing parsed configuration
  std::auto_ptr<USSGateway_XCFG> stCfg;

public:
  static const uint32_t   _MAX_THREADS_NUM = 0xFFFF;
  static const uint16_t   _DFLT_THREADS_NUM = 64;

  static const uint32_t   _MAX_REQUESTS_NUM = 0x7FFFFFFF;
  static const uint32_t   _DFLT_REQUESTS_NUM = 1000;

  static const uint32_t   _DFLT_CLIENT_CONNS = 5;

  USSGCfgReader(const char * nm_uss_srv, Logger * use_log)
      : nmSrv(nm_uss_srv), logger(use_log)
  { }
  ~USSGCfgReader()
  { }

  void readConfig(Config & root_sec) throw(ConfigException);

  USSGateway_XCFG * rlseConfig(void) { return stCfg.release(); }
  const USSGateway_XCFG * getConfig(void) const { return stCfg.get(); }
};

} //ussman
} //smsc

#endif /* __SMSC_USSMAN_CFG_READER_HPP__ */

