/* ************************************************************************** *
 * TCAP ABORT message (according to
 * itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)).
 * ************************************************************************** */
#ifndef __TCAP_MESSAGE_ABORT_DEFS_HPP
#ident "@(#)$Id$"
#define __TCAP_MESSAGE_ABORT_DEFS_HPP

#include "eyeline/tcap/proto/TCDlgPortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

/* AbortReason type is defined in IMPLICIT tagging environment as follow:
AbortReason ::= CHOICE {
    p-abortCause  P-AbortCause,
    u-abortCause  DialoguePortion
}
-- NOTE - When the Abort Message is generated by the Transaction sublayer,
-- a p-Abort Cause may be present.  The u-abortCause may be generated by the
-- component sublayer in which case it is either an ABRT APDU (if TCAP dilogue
-- is already established, or AARE APDU if dialogue association is requested
-- by previously received TR_Begin_Req), or by the TC-User in which case it
-- could be either an ABRT APDU or data in some user-defined abstract syntax.
*/
class TAbortReason : public util::ChoiceOf2_T<PAbort::Cause_t, TCDlgPortion> {
public:
  TAbortReason() : util::ChoiceOf2_T<PAbort::Cause_t, TCDlgPortion>()
  { }
  ~TAbortReason()
  { }

  bool isProviderCause(void) const { return getChoiceIdx() == 0; }
  bool isUserCause(void)     const { return getChoiceIdx() == 1; }

  Alternative_T<PAbort::Cause_t>  prvd() { return alternative0(); }
  Alternative_T<TCDlgPortion>     user() { return alternative1(); }

  ConstAlternative_T<PAbort::Cause_t>  prvd() const { return alternative0(); }
  ConstAlternative_T<TCDlgPortion>     user() const { return alternative1(); }

  //Verifies that u-abortCause contains allowed Structured Dialogue PDU or EXTERNAL
  bool verifyPdu(void) const
  {
    return (user().get()
            && (user().get()->empty()
                || (!user().get()->isUniDialoguePdu() 
                    && !user().get()->dlg().get()->aarq().get())
                )
            );
  }

  //Returns AARE_APdu, in case of abort of dialogue association request
  const TCPduAARE * getAARE(void) const
  {
    return (user().get() && user().get()->dlg().get()) ?
                user().get()->dlg().get()->aare().get() : 0;
  }
  //Returns ABRT_APdu, in case of abort of already established dialogue
  const TCPduABRT * getABRT(void) const
  {
    return (user().get() && user().get()->dlg().get()) ?
                user().get()->dlg().get()->abrt().get() : 0;
  }
  //Returns user defined data, in case of abort of already established dialogue
  const TDlgUserInfoList * getEXT(void) const
  {
    return (user().get() && user().get()->ext().get()) ? user().get()->ext().get() : 0;
  }


  //Returns AARE_APdu, in case of abort of dialogue association request
  TCPduAARE * getAARE(void)
  {
    return (user().get() && user().get()->dlg().get()) ? 
                user().get()->dlg().get()->aare().get() : 0;
  }
  //Returns ABRT_APdu, in case of abort of already established dialogue
  TCPduABRT * getABRT(void)
  {
    return (user().get() && user().get()->dlg().get()) ?
                user().get()->dlg().get()->abrt().get() : 0;
  }
  //Returns user defined data, in case of abort of already established dialogue
  TDlgUserInfoList * getEXT(void)
  {
    return (user().get() && user().get()->ext().get()) ? user().get()->ext().get() : 0;
  }

};

/* Abort message is defined in IMPLICIT tagging environment as follow:
Abort ::= [APPLICATION 7] SEQUENCE {
  dtid    DestTransactionID,
  reason  AbortReason OPTIONAL
} */
struct TMsgAbort {
  uint32_t      _dstTrId;
  TAbortReason  _reason; //Optional

  TMsgAbort() : _dstTrId(0)
  { }
  ~TMsgAbort()
  { }

  bool isByProvider(void) const { return _reason.isProviderCause(); }
  bool isByUser(void)     const { return _reason.isUserCause(); }

  //Verifies that u-abortCause contains allowed Structured Dialogue PDU or EXTERNAL
  bool verifyPdu(void) const
  {
    return _reason.empty() || _reason.verifyPdu();
  }

  //Returns AARE_APdu, in case of abort of dialogue association request
  const TCPduAARE * getAARE(void) const
  {
    return _reason.empty() ? _reason.getAARE() : 0;
  }
  //Returns ABRT_APdu, in case of abort of already established dialogue
  const TCPduABRT * getABRT(void) const
  {
    return _reason.empty() ? _reason.getABRT() : 0;
  }
  //Returns user defined data, in case of abort of already established dialogue
  const TDlgUserInfoList * getEXT(void) const
  {
    return _reason.empty() ? _reason.getEXT() : 0;
  }

  //Returns AARE_APdu, in case of abort of dialogue association request
  TCPduAARE * getAARE(void)
  {
    return _reason.empty() ? _reason.getAARE() : 0;
  }
  //Returns ABRT_APdu, in case of abort of already established dialogue
  TCPduABRT * getABRT(void)
  {
    return _reason.empty() ? _reason.getABRT() : 0;
  }
  //Returns user defined data, in case of abort of already established dialogue
  TDlgUserInfoList * getEXT(void)
  {
    return _reason.empty() ? _reason.getEXT() : 0;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_ABORT_DEFS_HPP */

