/* ************************************************************************** *
 * Helpers: Primitive Character Separated Values List parser.
 * ************************************************************************** */
#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/csv/CSVParser.hpp"

namespace smsc {
namespace util {
namespace csv {

const char * CSVParserAC::_DFLT_BLANKS_PATTERN = " \t\r\n";

//Returns number of splitted values
unsigned CSVParserAC::split(const char * str) /*throw(std::exception)*/
{
  if (!str || !str[0])
    return 0;

  std::string org_str(str);
  if (_cutBS && str_cut_blanks(org_str, _cutBS).empty())
    return 0;

  unsigned                numVals = 0;
  std::string             spVal; //value is to split
  std::string::size_type  pos = 0, dlmPos;
  do {
    dlmPos = org_str.find_first_of(_dlm, pos);
    {
      spVal.assign(org_str.c_str() + pos, ((dlmPos != org_str.npos) ? dlmPos : org_str.size()) - pos);
      if (_cutBS)
          str_cut_blanks(spVal, _cutBS);
      processValue(spVal);
    }
    ++numVals;
    pos = dlmPos + 1;
  } while (dlmPos != org_str.npos);

  return numVals;
}

//Returns number of values, which would be splitted from given string
unsigned CSVParserAC::estimate(const char * str) const /*throw(std::exception)*/
{
  if (!str || !str[0])
    return 0;

  std::string org_str(str);
  if (_cutBS && str_cut_blanks(org_str, _cutBS).empty())
    return 0;

  unsigned                numVals = 0;
  std::string::size_type  pos = 0, dlmPos;
  do {
    dlmPos = org_str.find_first_of(_dlm, pos);
    ++numVals;
    pos = dlmPos + 1;
  } while (dlmPos != org_str.npos);

  return numVals;
}

} //csv
} //util
} //smsc

