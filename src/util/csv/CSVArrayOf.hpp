/* ************************************************************************** *
 * Helpers: Character Separated Values container template : array
 * ************************************************************************** */
#ifndef __SMSC_UTIL_CSV_ARRAY_HPP
#ident "@(#)$Id$"
#define __SMSC_UTIL_CSV_ARRAY_HPP

#include <vector>
#include "util/csv/CSVParser.hpp"
#include "util/csv/CSValueTraits.hpp"

namespace smsc {
namespace util {
namespace csv {

template <
  class _TArg
>
class CSVArrayOf_T : public std::vector<_TArg>, protected CSVParserAC {
protected:
  // --------------------------------------------
  // -- CSVParserAC interface methods
  // --------------------------------------------
  virtual void processValue(const std::string & use_val)
    /*throw(std::exception)*/
  {
    push_back(CSValueTraits_T<_TArg>::str2val(use_val));
  }

public:
  typedef typename std::vector<_TArg>::size_type  size_type;
  typedef typename std::vector<_TArg>::iterator   iterator;
  typedef typename std::vector<_TArg>::const_iterator  const_iterator;

  CSVArrayOf_T(char use_dlm = ',',
          const char * cut_blanks = CSVParserAC::_DFLT_BLANKS_PATTERN)
    : std::vector<_TArg>(), CSVParserAC(use_dlm, cut_blanks)
  { }
  CSVArrayOf_T(size_type num_elem, char use_dlm = ',',
          const char * cut_blanks = CSVParserAC::_DFLT_BLANKS_PATTERN)
    : std::vector<_TArg>(num_elem), CSVParserAC(use_dlm, cut_blanks)
  { }
  ~CSVArrayOf_T()
  { }

  size_type fromStr(const char * str) /*throw(std::exception)*/
  {
    clear();
    return (size_type)CSVParserAC::split(str);
  }

  //Returns number of values, which would be splitted from given string.
  size_type estimate(const char * str) const /*throw(std::exception)*/
  {
    return (size_type)CSVParserAC::estimate(str);
  }

  //Appends string representing CSVList to given string
  size_type toString(std::string & out_str, char ins_space = ' ') const  /*throw()*/
  {
    if (empty())
      return 0;

    char  dlmStr[3];

    dlmStr[0] = getDelimiter();
    if (ins_space) {
      dlmStr[1] = ins_space; dlmStr[2] = 0;
    } else
      dlmStr[1] = 0;

    size_type      i = 0;
    const_iterator it = CSVArrayOf_T::begin();
    out_str += CSValueTraits_T<_TArg>::val2str(*it);
    while ((++it) != CSVArrayOf_T::end()) {
      out_str += dlmStr;
      out_str += CSValueTraits_T<_TArg>::val2str(*it);
      ++i;
    }
    return i;
  }

  std::string toString(char ins_space = ' ') const /*throw()*/
  {
    std::string elems;
    toString(elems, ins_space);
    return elems;
  }
};

typedef CSVArrayOf_T<std::string> CSVArrayOfStr;

} //csv
} //util
} //smsc

#endif /* __SMSC_UTIL_CSV_ARRAY_HPP */

