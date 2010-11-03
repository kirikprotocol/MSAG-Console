/* ************************************************************************** *
 * Configurable Services Host config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_SVCHOST_CONFIG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_SVCHOST_CONFIG_PARSING__

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/ICSRegistry.hpp"
#include "inman/services/SvcHostDefs.hpp"

namespace smsc {
namespace inman {

class ICSHostCfgReader : public ICSrvCfgReaderAC_T<SvcHostCFG> {
private:
  bool markProducer(ICSUId ics_uid);

  //Returns true if at least one new service was loaded
  bool addDfltLoadUps(void) /*throw(ConfigException)*/;

  //Reads 'Services' section containing services loadup configuration
  //NOTE: default config section names for services may be overridden
  //      only in 'Services' section
  void readLoadUps(XConfigView & cfg_sec) /*throw(ConfigException)*/;

  //Reads or complete reading the configuration of loaded services,
  //also loads up services requested by dependencies
  void readLoadedCfg(void);

protected:
  static const unsigned int _DFLT_CLIENT_CONNS = 3;

  //Parses XML configuration entry section, updates dependencies.
  //Returns status of config parsing, 
  CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException);

public:
  ICSHostCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
    : ICSrvCfgReaderAC_T<SvcHostCFG>(root_sec, use_log, ics_sec)
  { }
  ~ICSHostCfgReader()
  { }
};


} //inman
} //smsc
#endif /* __INMAN_SVCHOST_CONFIG_PARSING__ */

