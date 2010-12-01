/* ************************************************************************** *
 * XML Configuration Files Manager: lightweight implementation that just
 * operates with XML files and creates 'Config' objects.
 * ************************************************************************** */
#ifndef __UTIL_XCFG_MANAGER_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __UTIL_XCFG_MANAGER_HPP__

#include <sys/stat.h>

#include "util/config/Config.h"
#include "util/xml/init.h"

namespace smsc {
namespace util {
namespace config {

class XCFManager {
private:
  CStrSet path;

protected:
  XCFManager()
  { 
    smsc::util::xml::initXerces();
    path.insert(std::string("./"));
    path.insert(std::string("./conf/"));
    path.insert(std::string("../conf/"));
  }
  ~XCFManager()
  { 
    smsc::util::xml::TerminateXerces();
  }

public:
  static XCFManager & getInstance()
  {
    static XCFManager instance;
    return instance;
  }

  void addPath(const char * use_path);

  //Searches for file with given name according to configured path.
  //Returns actual file path(absolute or relative to working directory)
  bool findFile(const char * nm_file, std::string * rel_nm = NULL);

  //Parses XML configuration stored in file with given name
  void parseFile(Config & out_cfg, const char * nm_file_abs)
      throw(ConfigException);

  //Searches for XML configuration file with given name according to
  //configured path and parses it.
  //If requested (real_nm != 0), returns actual file path.
  void getConfig(Config & out_cfg, const char * xcfg_file, std::string * real_nm = NULL)
    throw(ConfigException);

  //Searches for XML configuration file with given name according to
  //configured path and parses it.
  //If requested (real_nm != 0), returns actual file path.
  //Returns newly allocated Config.
  Config * getConfig(const char * xcfg_file, std::string * real_nm = NULL)
    throw(ConfigException);
};

} //config
} //util
} //smsc
#endif /* __UTIL_XCFG_MANAGER_HPP__ */

