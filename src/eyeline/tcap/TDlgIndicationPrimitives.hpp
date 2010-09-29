/* ************************************************************************** *
 * TCAP API: structured TCAP dialogue indications primitives.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__

#include "eyeline/sccp/SCCPConsts.hpp"
#include "eyeline/sccp/SCCPAddress.hpp"
#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/tcap/TDlgUserInfo.hpp"
#include "eyeline/tcap/TComponentDefs.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;
using eyeline::sccp::SCCPAddress;

class TDlgIndicationPrimitive {
public:
  enum IKind_e {
    indTRNone = 0, indTRBegin, indTRCont, indTREnd, indTRUAbort, indTRPAbort, indTRNotice
  };

  virtual ~TDlgIndicationPrimitive()
  { }

  static const char * getIdent(IKind_e ind_kind);

  const char * getIdent(void) const { return getIdent(_iKind); }
  //
  TDialogueId getDialogueId(void) const { return _dlgId; }
  //
  const EncodedOID * getAppCtx(void) const { return _acOId; }
  //
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  //
  const TDlgUserInfoList * getUserInfo(void) const { return _usrInfo; }
  const TComponentsList * getCompList(void) const { return _comps; }
  //
  TDlgUserInfoList * getUserInfo(void) { return _usrInfo; }
  TComponentsList *  getCompList(void)  { return _comps; }

protected:
  const IKind_e       _iKind;
  TDialogueId         _dlgId;
  const EncodedOID *  _acOId;
  TComponentsList  *  _comps;
  TDlgUserInfoList *  _usrInfo;
  SCCPAddress         _orgAdr;

  TDlgIndicationPrimitive(IKind_e use_kind)
    : _iKind(use_kind), _dlgId(0), _acOId(0), _comps(0), _usrInfo(0)
  { }
};

//
class TR_Begin_Ind : public TDlgIndicationPrimitive {
protected:
  SCCPAddress   _dstAdr;

public:
  TR_Begin_Ind() : TDlgIndicationPrimitive(indTRBegin)
  { }
  ~TR_Begin_Ind()
  { }
  //
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
};
//
class TR_Cont_Ind : public TDlgIndicationPrimitive {
public:
  TR_Cont_Ind() : TDlgIndicationPrimitive(indTRCont)
  { }
  ~TR_Cont_Ind()
  { }
};
//
class TR_End_Ind : public TDlgIndicationPrimitive {
public:
  TR_End_Ind() : TDlgIndicationPrimitive(indTREnd)
  { }
  ~TR_End_Ind()
  { }
};

//
class TR_UAbort_Ind : public TDlgIndicationPrimitive {
protected:
  TDialogueAssociate::Diagnostic_t _diagnostic;

public:
  TR_UAbort_Ind() : TDlgIndicationPrimitive(indTRUAbort)
    , _diagnostic(0)
  { }
  ~TR_UAbort_Ind()
  { }

  TDialogueAssociate::Diagnostic_t getDiagnostic(void) const
  {
    return _diagnostic;
  }
};

//
class TR_PAbort_Ind : public TDlgIndicationPrimitive {
protected:
  PAbort::Cause_t _cause;

public:
  TR_PAbort_Ind() : TDlgIndicationPrimitive(indTRPAbort)
    , _cause(PAbort::p_genericError)
  { }
  ~TR_PAbort_Ind()
  { }
  //
  PAbort::Cause_t getCause(void) const { return _cause; }
  //
  bool islocal(void) const { return _orgAdr.empty(); }
};

//
class TR_Notice_Ind : public TDlgIndicationPrimitive {
protected:
  sccp::ReturnCause_t _cause;
  SCCPAddress         _dstAdr;
  uint16_t            _usrDataLen;
  const uint8_t *     _usrDataBuf;

public:
  TR_Notice_Ind() : TDlgIndicationPrimitive(indTRNotice)
    , _cause(sccp::ReturnCause::rcReserved), _usrDataLen(0), _usrDataBuf(0)
  { }
  ~TR_Notice_Ind()
  { }
  //
  sccp::ReturnCause_t getReturnCause(void) const { return _cause; }
  //
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__ */

