/* ************************************************************************** *
 * TCAP messages according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __TCAP_MESSAGE_DEFS_HPP

#include "eyeline/tcap/proto/TMsgUnidir.hpp"
#include "eyeline/tcap/proto/TMsgBegin.hpp"
#include "eyeline/tcap/proto/TMsgEnd.hpp"
#include "eyeline/tcap/proto/TMsgContinue.hpp"
#include "eyeline/tcap/proto/TMsgAbort.hpp"

#include "eyeline/util/ChoiceOfT.hpp"

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
class TCMessage : public util::ChoiceOf5_T<TMsgUnidir,
                    TMsgBegin, TMsgEnd, TMsgContinue, TMsgAbort> {
public:
  enum TMKind_e {
    t_none = -1
    //uniDialogue-a messages:
    , t_unidir = 0
    //dialogue-as messages:
    , t_begin = 1
    , t_end = 2
    , t_continue = 3
    , t_abort = 4
  };

  TCMessage()
  { }
  ~TCMessage()
  { }

  TMKind_e getKind(void) const { return static_cast<TMKind_e>(getChoiceIdx()); }

  Alternative_T<TMsgUnidir>   unidir()  { return alternative0(); }
  Alternative_T<TMsgBegin>    begin()   { return alternative1(); }
  Alternative_T<TMsgEnd>      end()     { return alternative2(); }
  Alternative_T<TMsgContinue> cont()    { return alternative3(); }
  Alternative_T<TMsgAbort>    abort()   { return alternative4(); }

  ConstAlternative_T<TMsgUnidir>   unidir() const { return alternative0(); }
  ConstAlternative_T<TMsgBegin>    begin()  const { return alternative1(); }
  ConstAlternative_T<TMsgEnd>      end()    const { return alternative2(); }
  ConstAlternative_T<TMsgContinue> cont()   const { return alternative3(); }
  ConstAlternative_T<TMsgAbort>    abort()  const { return alternative4(); }

  //Verifies that dialogue portion contains allowed PDU
  bool verifyPdu(void) const
  {
    switch (getKind()) {
    case t_unidir:
      unidir().get()->verifyPdu(); break;
    case t_begin:
      begin().get()->verifyPdu(); break;
    case t_end:
      end().get()->verifyPdu(); break;
    case t_continue:
      cont().get()->verifyPdu(); break;
    case t_abort:
      abort().get()->verifyPdu(); break;
    default:;
    } //eosw
    return true;
  }
};


} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_DEFS_HPP */

