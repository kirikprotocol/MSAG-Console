/* ************************************************************************** *
 * TCAP End message (according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_END_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_END_DEFS_HPP

#include "eyeline/tcap/TComponentDefs.hpp"
#include "eyeline/tcap/proto/TCDlgPortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

/* End message is defined in IMPLICIT tagging environment as follow:
End ::= [APPLICATION 4] SEQUENCE {
    dtid             DestTransactionID,
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
}
-- NOTE: When the dialoguePortion is present it may contain TCAP structured
--       dialogue AARE-APdu
*/
struct TMsgEnd {
  uint32_t        _dstTrId;
  TCDlgPortion    _dlgPart; //Optional
  TComponentsList _compPart;

  TMsgEnd() : _dstTrId(0)
  { }
  ~TMsgEnd()
  { }

  //Verifies that dialoguePortion contains allowed Structured Dialogue PDU
  bool verifyPdu(void) const
  {
    return (_dlgPart.empty() || (_dlgPart.dlg().get() && _dlgPart.dlg().get()->aare().get()));
  }

  const TCPduAARE * getAARE(void) const
  {
    return _dlgPart.dlg().get()  ? _dlgPart.dlg().get()->aare().get() : NULL;
  }

  TCPduAARE * getAARE(void)
  {
    return _dlgPart.dlg().get()  ? _dlgPart.dlg().get()->aare().get() : NULL;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_END_DEFS_HPP */

