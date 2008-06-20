#include <ctype.h>
#include <string>

namespace utilx {

const std::string&
toLowerCaseString(std::string& strValue)
{
  for(std::string::iterator iter = strValue.begin(), end_iter = strValue.end();
      iter != end_iter; ++iter) *iter = tolower(*iter);
  return strValue;
}

}
