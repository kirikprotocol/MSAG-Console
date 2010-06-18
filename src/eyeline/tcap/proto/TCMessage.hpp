/* ************************************************************************** *
 * TCAP messages according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_DEFS_HPP

#include "eyeline/tcap/proto/TMsgUnidir.hpp"
#include "eyeline/tcap/proto/TMsgBegin.hpp"
#include "eyeline/tcap/proto/TMsgEnd.hpp"
#include "eyeline/tcap/proto/TMsgContinue.hpp"
#include "eyeline/tcap/proto/TMsgAbort.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

/* According to Q.773, TCMessage is defined in IMPLICIT tagging
   environment as follow:

TCMessage ::= CHOICE {
  unidirectional    Unidirectional,
  begin             Begin,
  end               End,
  continue          Continue,
  abort             Abort
} */
class TCMessage {
public:
  enum TMKind_e {
    t_none = 0
    //uniDialogue-a messages:
    , t_unidir = 1    //[APPLICATION 1]
    //dialogue-as messages:
    , t_begin = 2     //[APPLICATION 2]
    , t_end = 4       //[APPLICATION 4]
    , t_continue = 5  //[APPLICATION 5]
    , t_abort = 7     //[APPLICATION 7]
  };

private:
  union {
    void *  _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf5_T<TMsgUnidir,
                    TMsgBegin, TMsgEnd, TMsgContinue, TMsgAbort>::VALUE];
  } _memMsg;

protected:
  TMKind_e  _kind;
  union {
    void *          _any;
    TMsgUnidir *    _unidir;
    TMsgBegin *     _begin;
    TMsgEnd *       _end;
    TMsgContinue *  _cont;
    TMsgAbort *     _abort;
  }         _msg;

public:
  explicit TCMessage() : _kind(t_none)
  {
    _msg._any = NULL;
  }
  ~TCMessage()
  {
    clear();
  }

  TMKind_e getKind(void) const { return _kind; }

  bool empty(void) const { return _kind == t_none; }

  void clear(void)
  {
    if (_msg._any) {
      switch (_kind) {
      case t_unidir:
        _msg._unidir->~TMsgUnidir(); break;
      case t_begin:
        _msg._begin->~TMsgBegin(); break;
      case t_end:
        _msg._end->~TMsgEnd(); break;
      case t_continue:
        _msg._cont->~TMsgContinue(); break;
      case t_abort:
        _msg._abort->~TMsgAbort(); break;
      default:;
      } //eosw
      _msg._any = NULL;
      _kind = t_none;
    }
  }

  //Verifies that dialogue portion contains allowed PDU
  bool verifyPdu(void) const
  {
    if (_msg._any) {
      switch (_kind) {
      case t_unidir:
        _msg._unidir->verifyPdu(); break;
      case t_begin:
        _msg._begin->verifyPdu(); break;
      case t_end:
        _msg._end->verifyPdu(); break;
      case t_continue:
        _msg._cont->verifyPdu(); break;
      case t_abort:
        _msg._abort->verifyPdu(); break;
      default:;
      } //eosw
    }
    return true;
  }

  TMsgUnidir *    getUnidir(void)   { return _kind == t_unidir ? _msg._unidir : 0; }
  TMsgBegin *     getBegin(void)    { return _kind == t_begin ? _msg._begin : 0; }
  TMsgEnd *       getEnd(void)      { return _kind == t_end ? _msg._end : 0; }
  TMsgContinue *  getContinue(void) { return _kind == t_continue ? _msg._cont : 0; }
  TMsgAbort *     getAbort(void)    { return _kind == t_abort ? _msg._abort : 0; }

  const TMsgUnidir *    getUnidir(void)   const { return _kind == t_unidir ? _msg._unidir : 0; }
  const TMsgBegin *     getBegin(void)    const { return _kind == t_begin ? _msg._begin : 0; }
  const TMsgEnd *       getEnd(void)      const { return _kind == t_end ? _msg._end : 0; }
  const TMsgContinue *  getContinue(void) const { return _kind == t_continue ? _msg._cont : 0; }
  const TMsgAbort *     getAbort(void)    const { return _kind == t_abort ? _msg._abort : 0; }


  TMsgUnidir &    initUnidir(void)
  {
    clear();
    _msg._unidir = new (_memMsg._buf) TMsgUnidir();
    return *_msg._unidir;
  }
  TMsgBegin &     initBegin(void)
  {
    clear();
    _msg._begin = new (_memMsg._buf) TMsgBegin();
    return *_msg._begin;
  }
  TMsgEnd &       initEnd(void)
  {
    clear();
    _msg._end = new (_memMsg._buf) TMsgEnd();
    return *_msg._end;
  }
  TMsgContinue &  initContinue(void)
  {
    clear();
    _msg._cont = new (_memMsg._buf) TMsgContinue();
    return *_msg._cont;
  }
  TMsgAbort &     initAbort(void)
  {
    clear();
    _msg._abort = new (_memMsg._buf) TMsgAbort();
    return *_msg._abort;
  }
};


} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_DEFS_HPP */

