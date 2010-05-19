/* ************************************************************************** *
 * XML Configuration File Section View - lightweight implementation, that
 * provides only read access to parameters/elements of 'Config' object.
 * ************************************************************************** */
/* NOTE: 'string' type parameteres are returned as 'const char *' instead of
 * newly allocated string.
 * ************************************************************************** */
#ifndef __UTIL_XCONFIG_VIEW_HPP__
#ident "@(#)$Id$"
#define __UTIL_XCONFIG_VIEW_HPP__

#include "util/config/Config.h"

namespace smsc {
namespace util {
namespace config {

class XConfigView {
protected:
  Config &    config;
  std::string nmSec;  //name of section this view represents

public:
  XConfigView(Config & use_cfg, const char* sec_nm = NULL)
      : config(use_cfg)
  {
    if (sec_nm)
        nmSec = sec_nm;
  }
  ~XConfigView()
  { }

  //returns name of related config section
  const char * relSection(void) const { return nmSec.c_str(); }
  Config &     relConfig(void)  const { return config; }

  //composes absolute name of related section element
  std::string elementName(const char * const sub_nm) const
  {
    std::string elNm(nmSec);
    if (sub_nm && sub_nm[0]) {
      if (!nmSec.empty())
          elNm += '.';
      elNm += sub_nm;
    }
    return elNm;
  }

  bool    findSubSection(const char * const subs_nm) const
  {
    std::string subsNm = elementName(subs_nm);
    return config.findSection(subsNm.c_str());
  }
  //Returns allocated XConfigView()
  XConfigView* getSubConfig(const char* subs_nm, bool abs_name = false) const
  {
    if (abs_name)
      return new XConfigView(config, subs_nm);

    std::string subsNm = elementName(subs_nm);
    return new XConfigView(config, subsNm.c_str());
  }

  CStrSet * getSectionNames(void) const
  {
    return config.getChildSectionNames(relSection());
  }
  CStrSet * getShortSectionNames(void) const
  {
    return config.getChildShortSectionNames(relSection());
  }
  CStrSet * getIntParamNames(void) const
  {
    return config.getChildIntParamNames(relSection());
  }
  CStrSet * getBoolParamNames(void) const
  {
    return config.getChildBoolParamNames(relSection());
  }
  CStrSet * getStrParamNames(void) const
  {
    return config.getChildStrParamNames(relSection());
  }
  //
  int32_t getInt(const char* param) const
      throw (ConfigException)
  {
    std::string subsNm = elementName(param);
    int32_t result;
    try { result = config.getInt(subsNm.c_str());
    } catch (const HashInvalidKeyException & exc) {
        throw ConfigException("int config parameter missed: %s", subsNm.c_str());
    }
    return result;
  }
  //
  bool getBool(const char* param) const
      throw (ConfigException)
  {
    std::string subsNm = elementName(param);
    bool result;
    try { result = config.getBool(subsNm.c_str());
    } catch (const HashInvalidKeyException & exc) {
        throw ConfigException("bool config parameter missed: %s", subsNm.c_str());
    }
    return result;
  }
  //
  const char* getString(const char* param) const
      throw (ConfigException)
  {
    std::string subsNm = elementName(param);
    const char * result;
    try { result = (const char *)config.getString(subsNm.c_str());
    } catch (const HashInvalidKeyException & exc) {
        throw ConfigException("string config parameter missed: %s", subsNm.c_str());
    }
    return result;
  }
};

} //config
} //util
} //smsc
#endif /* __UTIL_XCONFIG_VIEW_HPP__ */

