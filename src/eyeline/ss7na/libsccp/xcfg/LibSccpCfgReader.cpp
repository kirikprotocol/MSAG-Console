#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ss7na/libsccp/xcfg/LibSccpCfgReader.hpp"

#include "util/strutil.hpp"

namespace eyeline  {
namespace ss7na {
namespace libsccp {

using smsc::util::config::CStrSet;

/* ************************************************************************* *
 * class LibSccpCfgReader implementation
 * ************************************************************************* */
void LibSccpCfgReader::readConfig(XConfigView * root_sec, SccpConfig & st_cfg)
    throw(ConfigException)
{
  if (!root_sec->findSubSection(_nmSec))
    throw ConfigException("'%s' section is missing", _nmSec);

  std::auto_ptr<XConfigView> cfgSec(root_sec->getSubConfig(_nmSec));

  const char * cstr = 0;

  //<param name="appId" type="string">
  st_cfg._appId.clear();
  try { cstr = cfgSec->getString("appId"); }
  catch (...) { cstr = 0; }

  if (cstr) {
    st_cfg._appId = cstr;
    smsc::util::str_cut_blanks(st_cfg._appId);
    if (st_cfg._appId.size() > 255)
      throw ConfigException("'%s.appId' parameter value is longer than 255 chars",
                            cfgSec->relSection());
  }
  if (st_cfg._appId.empty())
    throw ConfigException("'%s.appId' parameter is invalid or missing",
                          cfgSec->relSection());

  //<param name="traffic-mode" type="string">
  cstr = 0;
  try { cstr = cfgSec->getString("traffic-mode"); }
  catch (...) { cstr = 0; }

  if (cstr) {
    std::string tstr(cstr);
    smsc::util::str_cut_blanks(tstr);
    if (!tstr.empty()) {
      if (!strcmp("loadshare", tstr.c_str()))
        st_cfg._trafficMode = SccpConfig::trfLOADSHARE;
      else if (!strcmp("override", tstr.c_str()))
        st_cfg._trafficMode = SccpConfig::trfOVERRIDE;
      else
        cstr = 0;
    } else
      cstr = 0;
  }
  if (!cstr)
    throw ConfigException("'%s.traffic-mode' parameter is invalid or missing",
                          cfgSec->relSection());

  //<section name="links">
  if (!cfgSec->findSubSection("links"))
    throw ConfigException("'%s.links' subsection is missing", cfgSec->relSection());
  if (!readLinks(cfgSec.get(), st_cfg._links))
    throw ConfigException("'%s.links' subsection is invalid", cfgSec->relSection());
}

//return false if no links is defined
bool LibSccpCfgReader::readLinks(XConfigView * outer_sec,
                                SccpConfig::LinksArray & links_arr)
     throw(ConfigException)
{
  links_arr.clear();
  std::auto_ptr<XConfigView> cfgSec(outer_sec->getSubConfig("links"));
  std::auto_ptr<CStrSet> xLinks(cfgSec->getShortSectionNames());

  for (CStrSet::const_iterator it = xLinks->begin(); it != xLinks->end(); ++it)
    readLinkParms(cfgSec.get(), it->c_str(), links_arr);

  return !links_arr.empty();
}


void LibSccpCfgReader::readLinkParms(XConfigView * outer_sec, const char * nm_sec,
                                    SccpConfig::LinksArray & links_arr)
     throw(ConfigException)
{
  std::auto_ptr<XConfigView> cfgSec(outer_sec->getSubConfig(nm_sec));
  //<param name="host" type="string">
  const char * cstr = 0;
  try { cstr = cfgSec->getString("host"); }
  catch (...) { cstr = 0; }
  if (!cstr)
    throw ConfigException("'%s.host' parameter is invalid or missing", cfgSec->relSection());

  //<param name="port" type="int">
  int32_t tInt = 0;
  try { tInt = cfgSec->getInt("port"); }
  catch (...) { tInt = 0; }
  if (!tInt || tInt > (in_port_t)(-1))
    throw ConfigException("'%s.port' parameter is invalid or missing", cfgSec->relSection());

  links_arr.push_back(SccpConfig::LinkId(nm_sec, cstr, (in_port_t)tInt));
  return;
}

} //libsccp
} //ss7na
} //eyeline


