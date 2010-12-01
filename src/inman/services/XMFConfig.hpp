/* ************************************************************************** *
 * Multifile XML configuration definition and helpers.
 * ************************************************************************** */
#ifndef __INMAN_MULTIFILE_XCFG_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_MULTIFILE_XCFG_HPP

#include <string>
#include <map>

#include "util/config/XCFManager.hpp"
#include "util/config/XCFView.hpp"

namespace smsc {
namespace inman {

using smsc::util::config::XCFManager;
using smsc::util::config::XConfigView;
using smsc::util::config::Config;
using smsc::util::config::ConfigException;

//Basic pattern: substitute symbols are:
// '?' - 1 any symbol, allowed at any position in pattern
// '*' - [0-N] any symbols, allowed at last position in pattern
class StrPrefix : protected std::string {
public:
  StrPrefix() : std::string()
  { }
  StrPrefix(const char * str_prfx) : std::string()
  {
    *this = str_prfx;
  }
  ~StrPrefix()
  { }

  static bool validate(const char * str_prfx);

  static bool isMatch(const char * str_prfx, const char * str_val);

  bool isMatch(const char * str_val) const
  {
    return isMatch(this->c_str(), str_val);
  }

  bool operator< (const StrPrefix &cmp_obj) const
  {
    return *((std::string*)this) < (const std::string &)cmp_obj;
  }

  StrPrefix & operator=(const char * str_prfx);
};


typedef std::map<std::string /*_nmFile*/, Config> XCFConfigsMap;
typedef XCFConfigsMap::value_type   XCFConfig;

class XMFConfig {
protected:
  typedef std::map<StrPrefix /*_secPrfx*/, XCFConfig *> XCFSectionsMap;

  XCFManager &    _fMgr;
  XCFConfigsMap   _xcfMap;
  XCFSectionsMap  _secMap;

  XCFConfig * verifyConfig(XCFConfig * xcf_cfg) throw(ConfigException)
  {
    if (xcf_cfg && xcf_cfg->second.empty())
      _fMgr.parseFile(xcf_cfg->second, xcf_cfg->first.c_str()); //throws
    return xcf_cfg;
  }

  XCFConfig * getSectionFile(const char * nm_sec_abs = NULL) const;

public:
  explicit XMFConfig(XCFManager & use_fmgr) : _fMgr(use_fmgr)
  { }
  ~XMFConfig()
  { }

  //Assigns config file for sections with specifed prefix.
  //If prefix is omitted, the '*' is assumed (i.e. root section)
  //Returns NULL if prefix is a duplicate one.
  //Throws in case of file cann't be found.
  //NOTE: File will be parsed later as it'll be demanded.
  const XCFConfig * setSectionsConfig(const char * nm_file, const char * prefix_str = NULL)
    throw(ConfigException);

  //Assigns config file for sections with specifed prefix.
  //If prefix is omitted, the '*' is assumed (i.e. root section)
  //Performs file parsing.
  //Returns NULL if prefix is a duplicate one.
  //Throws in case of file cann't be found or its parsing is failed.
  const XCFConfig * parseSectionsConfig(const char * nm_file, const char * prefix_str = NULL)
    throw(ConfigException);

  bool hasPrefix(const StrPrefix & prefix_str) const
  {
    XCFSectionsMap::const_iterator cit = _secMap.find(prefix_str);
    return (cit != _secMap.end());
  }

  bool hasPrefix(const char * prefix_str) const
  {
    StrPrefix newPfx(prefix_str);
    return hasPrefix(newPfx);
  }

  //Returns config file containing section with given absolute name
  //if section name is omitted, the root section is assumed
  const XCFConfig * getSectionConfig(const char * nm_sec_abs = NULL) const
  {
    const XCFConfig * xcfCfg = getSectionFile(nm_sec_abs);
    return (!xcfCfg || xcfCfg->second.empty()) ? NULL : xcfCfg;
  }
  //Returns config file containing section with given absolute name.
  //Performs file parsing if it doesn't parsed yet.
  //if section name is omitted, the root section is assumed
  XCFConfig * getSectionConfig(const char * nm_sec_abs = NULL) throw(ConfigException)
  {
    return verifyConfig(getSectionFile(nm_sec_abs)); //throws
  }

  //Searches associated config file for a section with given absolute name.
  //NOTE: Performs file parsing if necessary.
  bool hasSection(const char * nm_sec_abs)  throw(ConfigException)
  {
    XCFConfig * cfgFile = getSectionConfig(nm_sec_abs); //throws
    return cfgFile ? cfgFile->second.findSection(nm_sec_abs) : false;
  }

  //Assignes given XConfigView to section with specified absolute name
  //if section name is omitted, the root section is assumed.
  void getSectionView(XConfigView & use_view, const char * nm_sec_abs = NULL) const
  {
    const XCFConfig * cfgFile = getSectionConfig(nm_sec_abs);
    if (!cfgFile)
      use_view.clear();
    else
      use_view.assign(cfgFile->second, nm_sec_abs);
  }
  //Assignes given XConfigView to section with specified absolute name
  //if section name is omitted, the root section is assumed.
  //Performs file parsing if it doesn't parsed yet.
  void getSectionView(XConfigView & use_view, const char * nm_sec_abs = NULL)
  {
    XCFConfig * cfgFile = getSectionConfig(nm_sec_abs);
    if (!cfgFile)
      use_view.clear();
    else
      use_view.assign(cfgFile->second, nm_sec_abs);
  }

  //Assignes given XConfigView to root config section
  void getRootView(XConfigView & use_view) const
  {
    getSectionView(use_view, NULL);
  }
  //Assignes given XConfigView to root config section
  //Performs file parsing if it doesn't parsed yet.
  void getRootView(XConfigView & use_view)
  {
    getSectionView(use_view, NULL);
  }
};


} //inman
} //smsc

#endif /* __INMAN_MULTIFILE_XCFG_HPP */

