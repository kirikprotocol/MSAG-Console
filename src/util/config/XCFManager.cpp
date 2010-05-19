#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "util/config/XCFManager.hpp"
#include "util/xml/DOMTreeReader.h"

namespace smsc {
namespace util {
namespace config {

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
  std::string fn;
  for (CStrSet::const_iterator pit = path.begin();
                              pit != path.end(); ++pit) {
    struct stat s;
    fn = *pit; fn += nm_file;

    if (!stat(fn.c_str(), &s) && !S_ISDIR(s.st_mode)) {
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
  std::string relFn;
  std::string *pFn = real_nm ? real_nm : &relFn;
  if (!findFile(xcfg_file, pFn))
    throw ConfigException("%s file not found", pFn->c_str());

  util::xml::DOMTreeReader reader;
  DOMElement *elem = NULL;
  try {
    DOMDocument * document = reader.read(pFn->c_str());
    if (document)
      elem = document->getDocumentElement();
  } catch (const util::xml::ParseException & exc) {
    throw ConfigException("%s parsing failed: %s", pFn->c_str(), exc.what());
  }
  if (!elem)
    throw ConfigException("%s parsing failed: no element", pFn->c_str());
  return new Config(*elem); //throws
}


} //config
} //util
} //smsc

