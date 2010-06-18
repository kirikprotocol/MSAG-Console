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
-- by previously received TC_Begin_Req), or by the TC-User in which case it
-- could be either an ABRT APDU or data in some user-defined abstract syntax.
*/
class TAbortReason {
public:
  enum Kind_e { causeNone = 0, causePrvd, causeUser };

private:
  union {
    void * _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf2_T<PAbort::Cause_t, TCDlgPortion>::VALUE];
  } _memAlt;

protected:
  Kind_e  _kind;
  union {
    PAbort::Cause_t * _prvd;
    TCDlgPortion   *  _user;
  }       _cause;

public:
  explicit TAbortReason() : _kind(causeNone)
  {
    _memAlt._aligner = _cause._prvd = NULL;
  }
  ~TAbortReason()
  {
    clear();
  }

  Kind_e getKind(void) const { return _kind; }

  bool empty(void) const { return _kind != causeNone; }

  PAbort::Cause_t & initPrvd(void)
  {
    clear();
    _kind = causePrvd;
    _cause._prvd = _memAlt._buf;
    return *_cause._prvd;
  }
  TCDlgPortion & initUser(void)
  {
    clear();
    _kind = causeUser;
    _cause._user = new (_memAlt._buf) TCDlgPortion();
    return *_cause._user;
  }

  void clear(void)
  {
    if (_cause._user) {
      if (_kind == causeUser)
        _cause._user->~TCDlgPortion();
      _cause._prvd = NULL;
      _kind = causeNone;
    }
  }

  PAbort::Cause_t * getPrvd(void)
  {
    return (_kind == causePrvd) ? _cause._prvd : 0;
  }
  TCDlgPortion * getUser(void)
  {
    return (_kind == causeUser) ? _cause._user : 0;
  }

  const PAbort::Cause_t * getPrvd(void) const
  {
    return (_kind == causePrvd) ? _cause._prvd : 0;
  }
  const TCDlgPortion * getUser(void) const
  {
    return (_kind == causeUser) ? _cause._user : 0;
  }

  //Verifies that u-abortCause contains allowed Structured Dialogue PDU or EXTERNAL
  bool verifyPdu(void) const
  {
    return (getUser()
            && (getUser()->empty()
                || ((getUser()->getKind() != TCDlgPortion::asUNI) 
                    && !getUser()->getDLG()->getAARQ())
                )
            );
  }

  //Returns AARE_APdu, in case of abort of dialogue association request
  const TCPduAARE * getAARE(void) const
  {
    return (getUser() && getUser()->getDLG()) ? getUser()->getDLG()->getAARE() : 0;
  }
  //Returns ABRT_APdu, in case of abort of already established dialogue
  const TCPduABRT * getABRT(void) const
  {
    return (getUser() && getUser()->getDLG()) ? getUser()->getDLG()->getABRT() : 0;
  }
  //Returns user defined data, in case of abort of already established dialogue
  const TDlgUserInfoList * getEXT(void) const
  {
    return (getUser() && getUser()->getEXT()) ? getUser()->getEXT() : 0;
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

  explicit TMsgAbort() : _dstTrId(0)
  { }
  ~TMsgAbort()
  { }

  bool isByProvider(void) const { return _reason.getKind() == TAbortReason::causePrvd; }
  bool isByUser(void) const { return _reason.getKind() == TAbortReason::causeUser; }

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
};

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_MESSAGE_ABORT_DEFS_HPP */

