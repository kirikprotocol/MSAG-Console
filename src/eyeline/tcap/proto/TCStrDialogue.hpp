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

namespace eyeline {
namespace tcap {
namespace proto {

extern const asn1::EncodedOID _ac_tcap_strDialogue_as;

//Base class for Structured Dialogue PDUs
class TCDlgPduAC {
public:
  enum PduKind_e { pduNone = -1, pduAARQ = 0, pduAARE = 1, pduABRT = 4 };

protected:
  PduKind_e _kind;

public:
  tcap::TDlgUserInfoList  _usrInfo;    //optional

  TCDlgPduAC(PduKind_e use_pdu = pduNone)
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

  explicit TCPduAARQ()
    : TCDlgPduAC(TCDlgPduAC::pduAARQ), _protoVer(_dfltProtocolVersion)
  { }
  TCPduAARQ(const asn1::EncodedOID & use_ctx)
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
  TCPduAARE(const asn1::EncodedOID & use_ctx) : TCDlgPduAC(TCDlgPduAC::pduAARE)
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


class TCStrDialoguePdu {
private:
  union {
    void * _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf3_T<TCPduAARQ, TCPduAARE, TCPduABRT>::VALUE];
  } _memPdu;

protected:
  union {
    TCDlgPduAC * _any;
    TCPduAARQ *  _aarq;
    TCPduAARE *  _aare;
    TCPduABRT *  _abrt;
  } _pdu;

  void cleanUp(void)
  {
    if (_pdu._any) {
      _pdu._any->~TCDlgPduAC();
      _pdu._any = 0;
    }
  }
public:
  TCStrDialoguePdu()
  {
    _pdu._any = 0;
  }
  ~TCStrDialoguePdu()
  {
    cleanUp();
  }

  TCDlgPduAC::PduKind_e getKind(void) const
  {
    return _pdu._any ? _pdu._any->getKind() : TCDlgPduAC::pduNone;
  }

  const TCDlgPduAC * get(void) const { return _pdu._any; }

  TCPduAARQ * getAARQ(void) { return _pdu._aarq; }
  TCPduAARE * getAARE(void) { return _pdu._aare; }
  TCPduABRT * getABRT(void) { return _pdu._abrt; }

  const TCPduAARQ * getAARQ(void) const { return _pdu._aarq; }
  const TCPduAARE * getAARE(void) const { return _pdu._aare; }
  const TCPduABRT * getABRT(void) const { return _pdu._abrt; }

  TCPduAARQ & initAARQ(void)
  {
    cleanUp();
    _pdu._aarq = new (_memPdu._buf)TCPduAARQ();
    return *_pdu._aarq;
  }
  TCPduAARE & initAARE(void)
  {
    cleanUp();
    _pdu._aare = new (_memPdu._buf)TCPduAARE();
    return *_pdu._aare;
  }
  TCPduABRT & initABRT(void)
  {
    cleanUp();
    _pdu._abrt = new (_memPdu._buf)TCPduABRT();
    return *_pdu._abrt;
  }
};

} //proto
} //tcap
} //eyeline

#endif /* __TC_STR_DIALOGUE_DEFS_HPP */

