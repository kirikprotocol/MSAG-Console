/* ************************************************************************** *
 * Helpers: Character Separated Values container template : list.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_CSV_LIST_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_CSV_LIST_HPP

#include <list>
#include "util/csv/CSVParser.hpp"
#include "util/csv/CSValueTraits.hpp"

namespace smsc {
namespace util {
namespace csv {

template <
  class _TArg
>
class CSVListOf_T : public std::list<_TArg>, protected CSVParserAC {
protected:
  // --------------------------------------------
  // -- CSVParserAC interface methods
  // --------------------------------------------
  virtual void processValue(const std::string & use_val)
    /*throw(std::exception)*/
  {
    this->push_back(CSValueTraits_T<_TArg>::str2val(use_val));
  }

public:
  typedef typename std::list<_TArg>::size_type  size_type;
  typedef typename std::list<_TArg>::iterator   iterator;
  typedef typename std::list<_TArg>::const_iterator  const_iterator;

  CSVListOf_T(char use_dlm = ',',
          const char * cut_blanks = CSVParserAC::_DFLT_BLANKS_PATTERN)
    : std::list<_TArg>(), CSVParserAC(use_dlm, cut_blanks)
  { }
  CSVListOf_T(const char * str, char use_dlm = ',',
          const char * cut_blanks = CSVParserAC::_DFLT_BLANKS_PATTERN)
    : std::list<_TArg>(), CSVParserAC(use_dlm, cut_blanks)
  { 
    CSVParserAC::split(str);
  }
  ~CSVListOf_T()
  { }

  size_type fromStr(const char * str) /*throw(std::exception)*/
  {
    this->clear();
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
    if (this->empty())
      return 0;

    char  dlmStr[3];

    dlmStr[0] = getDelimiter();
    if (ins_space) {
      dlmStr[1] = ins_space; dlmStr[2] = 0;
    } else
      dlmStr[1] = 0;

    size_type      i = 1;
    const_iterator it = CSVListOf_T::begin();
    out_str += CSValueTraits_T<_TArg>::val2str(*it);
    while ((++it) != CSVListOf_T::end()) {
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

typedef CSVListOf_T<std::string> CSVListOfStr;

} //csv
} //util
} //smsc

#endif /* __SMSC_UTIL_CSV_LIST_HPP */

