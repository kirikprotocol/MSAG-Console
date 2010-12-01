#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/XMFConfig.hpp"

namespace smsc  {
namespace inman {
/* ************************************************************************** *
 * class StrPrefix implementation:
 * ************************************************************************** */
StrPrefix & StrPrefix::operator=(const char * str_prfx)
{
  if (str_prfx) { //if prefix doesn't end with '*' add it
    std::string::operator =(str_prfx);
    if (!strchr(str_prfx, '*'))
      std::string::operator +=('*');
  } else
    std::string::clear();
  return *this;
}

bool StrPrefix::validate(const char * str_prfx)
{ //verify that '*' is a last symbol
  const char * pAsterisk = strchr(str_prfx, '*');
  return (!pAsterisk || (strlen(str_prfx) == (size_t)(++pAsterisk - str_prfx)));
}

bool StrPrefix::isMatch(const char * str_prfx, const char * str_val)
{
  size_t valLen = strlen(str_val);
  size_t valPos = 0;

  while (*str_prfx) {
    if (*str_prfx == '*')
      return true;
    if (*str_prfx == '?') {
      if (valPos >= valLen) //EOStr
        return false;
    } else if ((valPos >= valLen) || (str_val[valPos] != *str_prfx))
      return false;

    ++str_prfx;
    ++valPos;
  } /*eow*/
  return true;
}

/* ************************************************************************** *
 * class XMFConfig implementation:
 * ************************************************************************** */
const XCFConfig *
  XMFConfig::parseSectionsConfig(const char * nm_file, const char * prefix_str/* = NULL*/)
    throw(ConfigException)
{
  if (!prefix_str || !*prefix_str)
    prefix_str = "*";

  StrPrefix newPfx(prefix_str);

  if (hasPrefix(newPfx)) //check prefix uniqueness
    return false;

  std::string nmFile;
  if (!_fMgr.findFile(nm_file, &nmFile))
    throw ConfigException("file not found: %s", nm_file);

  //check file uniqueness
  XCFConfigsMap::iterator fit = _xcfMap.find(nmFile);
  if (fit == _xcfMap.end()) {
    _fMgr.parseFile(_xcfMap[nmFile], nmFile.c_str()); //throws
    fit = _xcfMap.find(nmFile);
  } else if (fit->second.empty())
    _fMgr.parseFile(fit->second, fit->first.c_str()); //throws

  return (_secMap[newPfx] = fit.operator->());
}

const XCFConfig *
  XMFConfig::setSectionsConfig(const char * nm_file, const char * prefix_str/* = NULL*/)
    throw(ConfigException)
{
  if (!prefix_str || !*prefix_str)
    prefix_str = "*";

  StrPrefix newPfx(prefix_str);

  if (hasPrefix(newPfx)) //check prefix uniqueness
    return false;

  std::string nmFile;
  if (!_fMgr.findFile(nm_file, &nmFile))
    throw ConfigException("file not found: %s", nm_file);

  //check file uniqueness
  XCFConfigsMap::iterator fit = _xcfMap.find(nmFile);
  if (fit == _xcfMap.end()) {
    _xcfMap[nmFile]; //create empty Config()
    fit = _xcfMap.find(nmFile);
  }
  return (_secMap[newPfx] = fit.operator->());
}


XCFConfig *
  XMFConfig::getSectionFile(const char * nm_sec_abs/* = NULL*/) const
{
  if (!_secMap.empty()) {
    if (!nm_sec_abs || !*nm_sec_abs) {
      StrPrefix secPfx("*");
      XCFSectionsMap::const_iterator cit = _secMap.find(secPfx);
      return (cit == _secMap.end()) ? NULL : cit->second;
    }

    XCFSectionsMap::const_iterator cit = _secMap.end();
    do {
      if ((--cit)->first.isMatch(nm_sec_abs))
        return cit->second;
    } while (cit != _secMap.begin());
  }
  return NULL;
}

} // namespace inman
} // namespace smsc

