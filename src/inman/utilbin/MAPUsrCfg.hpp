/* ************************************************************************** *
 * MAP user XML config parser.
 * ************************************************************************** */
#ifndef __INMAN_MAPUSER_CFG_READER_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_MAPUSER_CFG_READER_HPP__

#include "inman/inap/TCXCfgParser.hpp"

namespace smsc  {
namespace inman {
namespace inap {

struct MAPUsr_CFG {
  TCDsp_CFG   ss7;
  TCAPUsr_CFG usr;
};


class MAPUsrCfgReader {
private:
  const char *    nmUsr;
  std::auto_ptr<MAPUsr_CFG> stCfg; //structure containing parsed configuration
  Logger *        logger;

public:
  MAPUsrCfgReader(const char * nm_map_usr, Logger * use_log)
      : nmUsr(nm_map_usr), logger(use_log)
  { }
  ~MAPUsrCfgReader()
  { }

  void readConfig(Config & root_sec) throw(ConfigException)
  {
    XConfigView rCfg(root_sec);
    const char * cstr = NULL;
    try { cstr = rCfg.getString(nmUsr);
    } catch (const ConfigException & exc) { }
    if (!cstr || !cstr[0])
      throw ConfigException("parameter '%s' is invalid or missing!");

    stCfg.reset(new MAPUsr_CFG());

    TCAPUsrCfgParser    usrParser(logger, cstr);
    if (!root_sec.findSection(usrParser.nmCfgSection()))
      throw ConfigException("section %s' is missing!", usrParser.nmCfgSection());
    smsc_log_info(logger, "Reading settings from '%s' ..", usrParser.nmCfgSection());
    usrParser.readConfig(root_sec, stCfg->usr); //throws

    TCDspCfgParser  dspParser(logger);
    if (!root_sec.findSection(dspParser.nmCfgSection()))
      throw ConfigException("section %s' is missing!", dspParser.nmCfgSection());
    smsc_log_info(logger, "Reading settings from '%s' ..", dspParser.nmCfgSection());
    dspParser.readConfig(root_sec, stCfg->ss7); //throws
  }

  MAPUsr_CFG * rlseConfig(void) { return stCfg.release(); }
  MAPUsr_CFG * getConfig(void) const { return stCfg.get(); }
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_MAPUSER_CFG_READER_HPP__ */

