/* ************************************************************************** *
 * Helpers: Primitive parser of Character Separated Values List.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_CSV_LIST_PARSER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_CSV_LIST_PARSER_HPP

#include "util/strutil.hpp"
#include "util/vformat.hpp"

namespace smsc {
namespace util {
namespace csv {

class CSVParserAC {
private:
  char          _dlm;   //values delimiter character
  const char *  _cutBS; //pattern of blanks to erase

public:
  static const char * _DFLT_BLANKS_PATTERN; //" \t\r\n"

  //Set 'cut_blanks' to NULL, in order to avoid blanks processing
  CSVParserAC(char use_dlm = ',', const char * cut_blanks = _DFLT_BLANKS_PATTERN)
      : _dlm(use_dlm), _cutBS(cut_blanks)
  { }
  CSVParserAC(const char * str, char use_dlm = ',', const char * cut_blanks = _DFLT_BLANKS_PATTERN)
      : _dlm(use_dlm), _cutBS(cut_blanks)
  { 
    split(str);
  }
  virtual ~CSVParserAC()
  { }

  char getDelimiter(void) const { return _dlm; }

  //Splits given string according to specified delimiter.
  //Returns number of splitted values.
  unsigned split(const char * str) /*throw(std::exception)*/;

  //Returns number of values, which would be splitted from given string.
  unsigned estimate(const char * str) const /*throw(std::exception)*/;

  // --------------------------------------------
  // -- CSVParserAC interface methods
  // --------------------------------------------
  //Processes the next value extracted from original CSV list string.
  virtual void processValue(const std::string & use_val) 
    /*throw(std::exception)*/ = 0;
};

} //csv
} //util
} //smsc

#endif /* __SMSC_UTIL_CSV_LIST_PARSER_HPP */

