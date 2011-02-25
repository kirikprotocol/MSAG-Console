/* ************************************************************************** *
 * TCAP Message Dialogue portion definition.
 * ************************************************************************** */
#ifndef __TC_MSG_DIALOGUE_PORTION_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __TC_MSG_DIALOGUE_PORTION_HPP

#include "eyeline/tcap/TDlgUserInfo.hpp"
#include "eyeline/tcap/proto/TCStrDialogue.hpp"
#include "eyeline/tcap/proto/TCUniDialogue.hpp"

#include "eyeline/util/ChoiceOfT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

using eyeline::tcap::TDlgUserInfoList;

/* DialoguePortion type is defined in IMPLICIT tagging environment as follow:

DialoguePortion ::= [APPLICATION 11] EXPLICIT EXTERNAL

-- The dialogue portion carries the dialogue control PDUs as value of the
-- external data type. The direct reference should be set to {itu-t 
-- recommendation q 773 as(1) dialogue-as(1) version1(1)} if structured 
-- dialogue is used and to {itu-t recommendation q 773 as(1) unidialogue-as(2)
-- version1(1)} if unstructured dialogue is used.
*/
class TCDlgPortion : public 
  util::ChoiceOf3_T<TCStrDialoguePdu, TCPduAUDT, TDlgUserInfoList> {
public:
  //Contained AbstractSyntax id
  enum ASKind_e { asNone = -1, asDLG = 0, asUNI = 1, asEXT = 2 };

  TCDlgPortion()
    : util::ChoiceOf3_T<TCStrDialoguePdu, TCPduAUDT, TDlgUserInfoList>()
  { }
  ~TCDlgPortion()
  { }

  ASKind_e getKind(void) const { return static_cast<ASKind_e>(getChoiceIdx()); }

  bool isStrDialoguePdu(void) const { return getKind() == asDLG; }
  bool isUniDialoguePdu(void) const { return getKind() == asUNI; }
  bool isExternal(void) const { return getKind() == asEXT; }

  Alternative_T<TCStrDialoguePdu> dlg() { return alternative0(); }
  Alternative_T<TCPduAUDT>        uni() { return alternative1(); }
  Alternative_T<TDlgUserInfoList> ext() { return alternative2(); }

  ConstAlternative_T<TCStrDialoguePdu> dlg() const { return alternative0(); }
  ConstAlternative_T<TCPduAUDT>        uni() const { return alternative1(); }
  ConstAlternative_T<TDlgUserInfoList> ext() const { return alternative2(); }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_MSG_DIALOGUE_PORTION_HPP */

