/* ************************************************************************** *
 * TCAP Continue message (according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_CONTINUE_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_CONTINUE_DEFS_HPP

#include "eyeline/tcap/TComponentDefs.hpp"
#include "eyeline/tcap/proto/TCDlgPortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

/* Continue message is defined in IMPLICIT tagging environment as follow:
Continue ::= [APPLICATION 5] SEQUENCE {
    otid             OrigTransactionID,
    dtid             DestTransactionID,
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
}
-- NOTE: When the dialoguePortion is present it may contain TCAP structured
--       dialogue AARE-APdu
*/
struct TMsgContinue {
  uint32_t        _orgTrId;
  uint32_t        _dstTrId;
  TCDlgPortion    _dlgPart; //Optional
  TComponentsList _compPart;

  explicit TMsgContinue() : _orgTrId(0), _dstTrId(0)
  { }
  ~TMsgContinue()
  { }

  //Verifies that dialoguePortion contains allowed Structured Dialogue PDU
  bool verifyPdu(void) const
  {
    return (_dlgPart.empty() || (_dlgPart.getDLG() && _dlgPart.getDLG()->getAARE()));
  }
  const TCPduAARE * getAARE(void) const
  {
    return _dlgPart.getDLG()  ? _dlgPart.getDLG()->getAARE() : NULL;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_CONTINUE_DEFS_HPP */

