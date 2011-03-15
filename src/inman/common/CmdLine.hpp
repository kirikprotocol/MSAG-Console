/* ************************************************************************** *
 * Command line parsing helpers. Input string may contain nested quotes and 
 * escaped chars.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_CMDLINE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_CMDLINE_HPP

#include <vector>
#include <string>

namespace smsc {
namespace inman {
namespace common {

struct ParsingResult {
  enum Status_e {
    rcOk = 0
    , rcEscape  //invalid escape sequence
    , rcMatch   //mismathced <'> or <">
  };
  Status_e  _status;
  unsigned  _pos;     //position of last processed character.

  ParsingResult() : _status(rcOk), _pos(0)
  { }
  ParsingResult(Status_e use_status, unsigned use_pos) : _status(use_status), _pos(use_pos)
  { }

  bool isOk(void) const { return _status == rcOk; }

  ParsingResult & operator+=(const ParsingResult & cp_obj)
  {
    _status = cp_obj._status;
    _pos += cp_obj._pos;
    return *this;
  }
};

//Appends parsed arguments to given vector.
extern ParsingResult parseCmdLine(const char * in_buf, std::vector<std::string> & cmd_args);

} // common
} // inman
} // smsc

#endif /* __SMSC_INMAN_CMDLINE_HPP */

