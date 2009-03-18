#include <ctype.h>
#include <string>
#include "toLowerCaseString.hpp"

namespace eyeline {
namespace utilx {

const std::string&
toLowerCaseString(std::string& strValue)
{
  for(std::string::iterator iter = strValue.begin(), end_iter = strValue.end();
      iter != end_iter; ++iter) *iter = tolower(*iter);
  return strValue;
}

}}
