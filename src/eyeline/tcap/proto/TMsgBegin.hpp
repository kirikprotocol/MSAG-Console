/* ************************************************************************** *
 * TCAP Begin message (according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_BEGIN_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_BEGIN_DEFS_HPP

#include "eyeline/tcap/TComponentDefs.hpp"
#include "eyeline/tcap/proto/TCDlgPortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

/* Begin message is defined in IMPLICIT tagging environment as follow:
Begin ::= [APPLICATION 2] SEQUENCE {
    otid             OrigTransactionID,
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
}
-- NOTE: When the dialoguePortion is present it may contain TCAP structured
--       dialogue AARQ-APdu
*/
struct TMsgBegin {
  uint32_t        _orgTrId;
  TCDlgPortion    _dlgPart; //Optional
  TComponentsList _compPart;

  TMsgBegin() : _orgTrId(0)
  { }
  ~TMsgBegin()
  { }

  //Verifies that dialoguePortion contains allowed Structured Dialogue PDU
  bool verifyPdu(void) const
  {
    return (_dlgPart.empty() || (_dlgPart.dlg().get() && _dlgPart.dlg().get()->aarq().get()));
  }

  const TCPduAARQ * getAARQ(void) const
  {
    return _dlgPart.dlg().get()  ? _dlgPart.dlg().get()->aarq().get() : NULL;
  }

  TCPduAARQ * getAARQ(void)
  {
    return _dlgPart.dlg().get()  ? _dlgPart.dlg().get()->aarq().get() : NULL;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_BEGIN_DEFS_HPP */

