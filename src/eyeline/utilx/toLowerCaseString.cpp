#include <ctype.h>
#include <string>
#include <algorithm>
#include "toLowerCaseString.hpp"

namespace eyeline {
namespace utilx {

std::string
toLowerCaseString(std::string strValue)
{
  std::transform(strValue.begin(), strValue.end(), strValue.begin(), tolower);
  return strValue;
}

}}
