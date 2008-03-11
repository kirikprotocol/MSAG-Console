#pragma ident "$Id$"
/* ************************************************************************** *
 * TCAP Dispatcher service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_TCDSP_CONFIG_PARSING__
#define __INMAN_TCDSP_CONFIG_PARSING__

#include "inman/inap/TCXCfgParser.hpp"
#include "inman/services/ICSCfgReader.hpp"
using smsc::inman::ICSrvCfgReaderAC_T;

namespace smsc {
namespace inman {
namespace inap {

class ICSTCDispCfgReader : public ICSrvCfgReaderAC_T<TCDsp_CFG> {
protected:
    //Parses XML configuration entry section, updates dependencies.
    //Returns status of config parsing, 
    CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException)
    {
        TCDspCfgParser  parser(logger, nmCfgSection());
        parser.readConfig(rootSec, *icsCfg.get()); //throws
        /**/
        return ICSrvCfgReaderAC::cfgComplete;
    }

public:
    ICSTCDispCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
        : ICSrvCfgReaderAC_T<TCDsp_CFG>(root_sec, use_log, ics_sec ? ics_sec : "SS7")
    { }
    ~ICSTCDispCfgReader()
    { }
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCDSP_CONFIG_PARSING__ */

