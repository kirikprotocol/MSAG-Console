#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <list>
#include <string.h>

#include "inman/common/CmdLine.hpp"


namespace smsc {
namespace inman {
namespace common {

struct ArgDelimiter {
  static const unsigned _MAX_OPTS = 4;

  bool  _isMatched;
  char  _dlms[_MAX_OPTS];        //delimiters
  char  _dlmOpts[_MAX_OPTS];  //optional nested delimiters


  ArgDelimiter(bool is_matched, const char * use_dlms, const char * dlm_nested = NULL)
    : _isMatched(is_matched)
  {
    unsigned i = 0;
    for (; (i < _MAX_OPTS) && use_dlms[i]; ++i) {
      _dlms[i] = use_dlms[i];
    }
    _dlms[i] = 0;
    i = 0;
    if (dlm_nested) {
      for (; (i < _MAX_OPTS) && dlm_nested[i]; ++i)
        _dlmOpts[i] = dlm_nested[i];
    }
    _dlmOpts[i] = 0;
  }
};

typedef std::list<const ArgDelimiter *> DelimitersStack;

static const ArgDelimiter _argDlms[3] = {
    ArgDelimiter(false, " \t", "\"\'")  //_dlmSpace
  , ArgDelimiter(true, "\"", "\'")      //_dlmDoubleQuote
  , ArgDelimiter(true, "\'")            //_dlmQuote
};

static const ArgDelimiter * getDelimiter(char dlm_id)
{
  for (unsigned i = 0; i < sizeof(_argDlms)/sizeof(ArgDelimiter); ++i) {
    if (strchr(_argDlms[i]._dlms, dlm_id))
      return &_argDlms[i];
  }
  return NULL;
}


struct EscapedChar {
  char _idChar;
  char _symbol;

  EscapedChar(char use_id, char use_symbol)
    : _idChar(use_id), _symbol(use_symbol)
  { }
};

static const EscapedChar _escapedChars[3] = {
  EscapedChar('t', '\t'), EscapedChar('r', '\r'), EscapedChar('n', '\n')
};

static char getEscapedChar(char id_char)
{
  for (unsigned i = 0; i < sizeof(_escapedChars)/sizeof(EscapedChar); ++i) {
    if (_escapedChars[i]._idChar == id_char)
      return _escapedChars[i]._symbol;
  }
  return id_char;
}


static ParsingResult getCmdArgument(const char * in_buf, std::vector<std::string> & cmd_args,
                                    DelimitersStack & dlm_stack)
{
  bool            isEscaped = false;
  ParsingResult   res(ParsingResult::rcOk, 0);
  std::string *   argStr = &cmd_args.back();

  if (!argStr->empty()) {
    cmd_args.push_back(std::string());
    argStr = &cmd_args.back();
  }

  while (in_buf[res._pos] && !dlm_stack.empty()) {
    if (isEscaped) {
      *argStr += getEscapedChar(in_buf[res._pos]);
      isEscaped = false;
    } else if (in_buf[res._pos] == '\\') {
      isEscaped = true;
    } else {
      //check for possible nested delimiters
      if (strchr(dlm_stack.front()->_dlmOpts, in_buf[res._pos])) {
        const ArgDelimiter * nextDlm = getDelimiter(in_buf[res._pos]);
        dlm_stack.push_front(nextDlm); //shouldn't fail here
        /**/
      } else if (strchr(dlm_stack.front()->_dlms, in_buf[res._pos])) {
        //check matchness
        if (dlm_stack.front()->_isMatched)
          dlm_stack.pop_front();
        else
          break;
      } else
        *argStr += in_buf[res._pos];
    }
    ++res._pos;
  }
  if (isEscaped)
    res._status = ParsingResult::rcEscape;
  else if (!dlm_stack.empty() && dlm_stack.front()->_isMatched)
    res._status =  ParsingResult::rcMatch;
  return res;
}

ParsingResult parseCmdLine(const char * in_buf, std::vector<std::string> & cmd_args)
{
  DelimitersStack dlmStack;
  ParsingResult   res(ParsingResult::rcOk, 0);

  dlmStack.push_front(&_argDlms[0]);
  //cmd_args.clear();
  cmd_args.push_back(std::string());
  do {
    res += getCmdArgument(in_buf + res._pos, cmd_args, dlmStack);
    if (!in_buf[res._pos] || (res._status != ParsingResult::rcOk))
      break;
    res._pos += 1;
  } while (in_buf[res._pos]);

  //last arument may be empty -> erase it
  if (cmd_args.back().empty())
    cmd_args.pop_back();
  return res;
}

} //common
} //inman
} //smsc
