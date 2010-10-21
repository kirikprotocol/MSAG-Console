/* ************************************************************************** *
 * TCAP structured dialogue definitions according to
 * itu-t recommendation q 773 modules(2) dialoguePDUs(2) version1(1).
 * ************************************************************************** */
#ifndef __TC_STR_DIALOGUE_DEFS_HPP
#ident "@(#)$Id$"
#define __TC_STR_DIALOGUE_DEFS_HPP

#include "eyeline/asn1/EncodedOID.hpp"
#include "eyeline/tcap/TDlgUserInfo.hpp"
#include "eyeline/tcap/proto/TCAssociateDiagnostic.hpp"
#include "eyeline/tcap/proto/ProtocolVersion.hpp"

#include "eyeline/util/ChoiceOfT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

extern const asn1::EncodedOID _ac_tcap_strDialogue_as;

//Base class for Structured Dialogue PDUs
class TCDlgPduAC {
public:
  enum PduKind_e { pduNone = -1, pduAARQ = 0, pduAARE = 1, pduABRT = 2 };

protected:
  PduKind_e _kind;

public:
  tcap::TDlgUserInfoList  _usrInfo;    //optional

  explicit TCDlgPduAC(PduKind_e use_pdu = pduNone)
    : _kind(use_pdu)
  { }
  virtual ~TCDlgPduAC()
  { }

  PduKind_e getKind(void) const { return _kind; }
};

/* ********************************************** *
 * Structured TC Dialogue request APDU (AARQ)
 * ********************************************* */
class TCPduAARQ : public TCDlgPduAC {
public:
  ProtocolVersion   _protoVer;
  asn1::EncodedOID  _acId;

  TCPduAARQ()
    : TCDlgPduAC(TCDlgPduAC::pduAARQ), _protoVer(_dfltProtocolVersion)
  { }
  explicit TCPduAARQ(const asn1::EncodedOID & use_ctx)
    : TCDlgPduAC(TCDlgPduAC::pduAARQ), _protoVer(_dfltProtocolVersion)
    , _acId(use_ctx)
  { }
  ~TCPduAARQ()
  { }
};

/* ********************************************** *
 * Structured TC Dialogue response APDU (AARE)
 * ********************************************* */
class TCPduAARE : public TCDlgPduAC {
public:
  ProtocolVersion               _protoVer;
  asn1::EncodedOID              _acId;
  TDialogueAssociate::Result_t  _result;
  AssociateSourceDiagnostic     _diagnostic;

  TCPduAARE() : TCDlgPduAC(TCDlgPduAC::pduAARE)
    , _protoVer(_dfltProtocolVersion), _result(TDialogueAssociate::dlg_accepted)
  { }
  explicit TCPduAARE(const asn1::EncodedOID & use_ctx)
    : TCDlgPduAC(TCDlgPduAC::pduAARE)
    , _protoVer(_dfltProtocolVersion), _acId(use_ctx)
    , _result(TDialogueAssociate::dlg_accepted)
  { }
  ~TCPduAARE()
  { }

  void acceptByUser(void)
  {
    _result = TDialogueAssociate::dlg_accepted;
    _diagnostic.setUserDiagnostic();
  }
  void acceptByPrvd(void)
  {
    _result = TDialogueAssociate::dlg_accepted;
    _diagnostic.setPrvdDiagnostic();
  }
  void rejectByUser(AssociateSourceDiagnostic::DiagnosticUser_e use_cause =
                        AssociateSourceDiagnostic::dsu_null)
  {
    _result = TDialogueAssociate::dlg_reject_permanent;
    _diagnostic.setUserDiagnostic(use_cause);
  }
  void rejectByPrvd(AssociateSourceDiagnostic::DiagnosticProvider_e use_cause =
                        AssociateSourceDiagnostic::dsp_null)
  {
    _result = TDialogueAssociate::dlg_reject_permanent;
    _diagnostic.setPrvdDiagnostic(use_cause);
  }
};

/* ********************************************** *
 * Structured TC Dialogue abort APDU (ABRT)
 * ********************************************* */
class TCPduABRT : public TCDlgPduAC {
public:
  TDialogueAssociate::AbrtSource_t  _abrtSrc;

  explicit TCPduABRT(TDialogueAssociate::AbrtSource_e abrt_src
                      = TDialogueAssociate::abrtServiceProvider)
    : TCDlgPduAC(TCDlgPduAC::pduABRT), _abrtSrc(abrt_src)
  { }
  ~TCPduABRT()
  { }
};


class TCStrDialoguePdu : public
  util::ChoiceOfBased3_T<TCDlgPduAC, TCPduAARQ, TCPduAARE, TCPduABRT> {
public:
  TCStrDialoguePdu() : util::ChoiceOfBased3_T
    <TCDlgPduAC, TCPduAARQ, TCPduAARE, TCPduABRT>()
  { }
  ~TCStrDialoguePdu()
  { }

  TCDlgPduAC::PduKind_e getKind(void) const
  {
    return static_cast<TCDlgPduAC::PduKind_e>(getChoiceIdx());
  }

  Alternative_T<TCPduAARQ> aarq(void) { return alternative0(); }
  Alternative_T<TCPduAARE> aare(void) { return alternative1(); }
  Alternative_T<TCPduABRT> abrt(void) { return alternative2(); }

  ConstAlternative_T<TCPduAARQ> aarq(void) const { return alternative0(); }
  ConstAlternative_T<TCPduAARE> aare(void) const { return alternative1(); }
  ConstAlternative_T<TCPduABRT> abrt(void) const { return alternative2(); }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_STR_DIALOGUE_DEFS_HPP */

