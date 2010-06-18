/* ************************************************************************** *
 * TCAP Message Dialogue portion definition.
 * ************************************************************************** */
#ifndef __TC_MSG_DIALOGUE_PORTION_HPP
#ident "@(#)$Id$"
#define __TC_MSG_DIALOGUE_PORTION_HPP

#include "eyeline/asn1/ASExternal.hpp"
#include "eyeline/tcap/proto/TCStrDialogue.hpp"
#include "eyeline/tcap/proto/TCUniDialogue.hpp"

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
class TCDlgPortion {
public:
  //Contained AbstractSyntax id
  enum ASKind_e { asNone = 0, asDLG, asUNI, asEXT };

private:
  union {
    void * _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf3_T<TDlgUserInfoList, TCPduAUDT,
                                            TCStrDialoguePdu>::VALUE];
  } _memPdu;

protected:
  ASKind_e _kind;
  union {
    //NOTE: single EXTERNAL type value is stored in UserInfo list in order
    //      to ease intercation with abovelying layers!
    TDlgUserInfoList *  _ext; 
    TCStrDialoguePdu *  _dlg;
    TCPduAUDT *         _uni;
  } _as;

public:
  TCDlgPortion() : _kind(asNone)
  {
    _as._ext = 0;
  }
  ~TCDlgPortion()
  {
    clear();
  }

  void clear(void)
  {
    if (_as._ext) {
      switch (_kind) {
      case asDLG:
        _as._dlg->~TCStrDialoguePdu(); break;
      case asUNI:
        _as._uni->~TCPduAUDT(); break;
      default: //      case asEXT:
        _as._ext->~TDlgUserInfoList(); break;
      }
      _as._ext = 0;
      _kind = asNone;
    }
  }

  ASKind_e getKind(void) const { return _kind; }

  bool empty(void) const { return _kind != asNone; }

  bool isStrDialoguePdu(void) const { return _kind == asDLG; }
  bool isUniDialoguePdu(void) const { return _kind == asUNI; }
  bool isExternal(void) const { return _kind == asEXT; }

  TCPduAUDT *         getUNI(void) { return _kind == asUNI ? _as._uni : 0; }
  TCStrDialoguePdu *  getDLG(void) { return _kind == asDLG ? _as._dlg : 0; }
  TDlgUserInfoList *  getEXT(void) { return _kind == asEXT ? _as._ext : 0; }

  const TCPduAUDT *         getUNI(void) const { return _kind == asUNI ? _as._uni : 0; }
  const TCStrDialoguePdu *  getDLG(void) const { return _kind == asDLG ? _as._dlg : 0; }
  const TDlgUserInfoList *  getEXT(void) const { return _kind == asEXT ? _as._ext : 0; }

  TCStrDialoguePdu & initDLG(void)
  {
    clear();
    _as._dlg = new (_memPdu._buf)TCStrDialoguePdu();
    _kind = asDLG;
    return *_as._dlg;
  }
  TCPduAUDT & initUNI(void)
  {
    clear();
    _as._uni = new (_memPdu._buf)TCPduAUDT();
    _kind = asUNI;
    return *_as._uni;
  }
  TDlgUserInfoList & initEXT(void)
  {
    clear();
    _as._ext = new (_memPdu._buf)TDlgUserInfoList();
    _kind = asEXT;
    return *_as._ext;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_MSG_DIALOGUE_PORTION_HPP */

