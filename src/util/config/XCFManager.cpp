#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/config/XCFManager.hpp"
#include "util/xml/DOMTreeReader.h"

namespace smsc {
namespace util {
namespace config {

//Returns true if file with given exists ans is regular file
static bool statFile(const char * nm_file)
{
  struct stat s;
  return (!stat(nm_file, &s) && !S_ISDIR(s.st_mode) && S_ISREG(s.st_mode));
}

/* ************************************************************************* *
 * class XCFManager implementation
 * ************************************************************************* */
void XCFManager::addPath(const char * use_path)
{
  std::string fn(use_path);
  std::string::size_type bpos = fn.find_last_of('/');
  if (bpos == fn.npos || (bpos != fn.size()))
      fn += '/';
  path.insert(fn);
}

//returns actual file path, relating to working directory
bool XCFManager::findFile(const char * nm_file, std::string * rel_nm/* = NULL*/)
{
  if (nm_file[0] == '/') {
    if (statFile(nm_file)) {
      if (rel_nm)
        *rel_nm = nm_file;
      return true;
    }
    return false;
  }

  std::string fn;
  for (CStrSet::const_iterator pit = path.begin();
                              pit != path.end(); ++pit) {
    fn = *pit; fn += nm_file;
    if (statFile(fn.c_str())) {
      if (rel_nm)
        *rel_nm = fn;
      return true;
    }
  }
  return false;
}

//Searches for XML configuration file, perfoms its parsing.
//Returns allocated Config() on success.
Config * XCFManager::getConfig(const char * xcfg_file, std::string * real_nm/* = NULL*/)
    throw(ConfigException)
{
  std::auto_ptr<Config> pCfg(new Config());
  getConfig(*pCfg.get(), xcfg_file, real_nm); //throws
  return pCfg.release();
}

void XCFManager::getConfig(Config & out_cfg, const char * xcfg_file, std::string * real_nm/* = NULL*/)
    throw(ConfigException)
{
  std::string relFn;
  if (!findFile(xcfg_file, &relFn))
    throw ConfigException("%s file not found", xcfg_file);

  parseFile(out_cfg, relFn.c_str()); //throws
  if (real_nm)
    *real_nm = relFn;
}

void XCFManager::parseFile(Config & out_cfg, const char * nm_file_abs)
    throw(ConfigException)
{
  util::xml::DOMTreeReader reader(DOMError::DOM_SEVERITY_ERROR);
  DOMElement * elem = NULL;
  try {
    DOMDocument * document = reader.read(nm_file_abs);
    if (document)
      elem = document->getDocumentElement();
  } catch (const util::xml::ParseException & exc) {
    throw ConfigException("%s parsing failed: %s", nm_file_abs, exc.what());
  } catch (...) {
    throw ConfigException("%s parsing failed: <unknown exception>", nm_file_abs);
  }
  if (!elem)
    throw ConfigException("%s parsing failed: no element", nm_file_abs);

  out_cfg.parse(*elem); //throws
}

} //config
} //util
} //smsc

