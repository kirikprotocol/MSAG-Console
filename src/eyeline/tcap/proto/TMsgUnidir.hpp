/* ************************************************************************** *
 * TCAP Unidirectional message (according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_UNIDIR_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_UNIDIR_DEFS_HPP

#include "eyeline/tcap/TComponentDefs.hpp"
#include "eyeline/tcap/proto/TCDlgPortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

/* Unidirectional message is defined in IMPLICIT tagging environment as follow:
Unidirectional ::= [APPLICATION 1] SEQUENCE {
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
}
-- NOTE: When the dialoguePortion is present it may contain TCAP
--       unidialogue AUDT-APdu
*/
struct TMsgUnidir {
  TCDlgPortion    _dlgPart; //Optional
  TComponentsList _compPart;

  explicit TMsgUnidir()
  { }
  ~TMsgUnidir()
  { }

  //Verifies that u-abortCause contains allowed Structured Dialogue PDU or EXTERNAL
  bool verifyPdu(void) const
  {
    return _dlgPart.empty() || _dlgPart.getUNI();
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_UNIDIR_DEFS_HPP */

