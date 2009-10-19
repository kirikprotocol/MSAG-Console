/* ************************************************************************** *
 * Configurable Services Host config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_SVCHOST_CONFIG_PARSING__
#ident "@(#)$Id$"
#define __INMAN_SVCHOST_CONFIG_PARSING__

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/ICSRegistry.hpp"
#include "inman/services/SvcHostDefs.hpp"

namespace smsc {
namespace inman {

class ICSHostCfgReader : public ICSrvCfgReaderAC_T<SvcHostCFG> {
private:
  bool markProducer(ICSUId ics_uid);

  void addDfltLoadUps(void) /*throw(ConfigException)*/;

  //Reads 'Services' section containing services loadup configuration
  //NOTE: default config section names for services may be overridden
  //      only in 'Services' section
  void readLoadUps(XConfigView & cfg_sec) /*throw(ConfigException)*/;

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

