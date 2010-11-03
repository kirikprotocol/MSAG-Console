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

  //returns actual file path, relating to working directory
  bool findFile(const char * nm_file, std::string * rel_nm = NULL);

  //Searches for XML configuration file, perfoms its parsing.
  //Returns allocated Config() on success.
  Config * getConfig(const char * xcfg_file, std::string * real_nm = NULL)
      throw(ConfigException);
};

} //config
} //util
} //smsc
#endif /* __UTIL_XCFG_MANAGER_HPP__ */

