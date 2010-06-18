/* ************************************************************************** *
 * TCAP API: structured TCAP dialogue indications primitives.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__

# include "eyeline/sccp/SCCPConsts.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/TDialogueDefs.hpp"
# include "eyeline/tcap/TDlgUserInfo.hpp"
# include "eyeline/tcap/TComponentDefs.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;
using eyeline::sccp::SCCPAddress;

class TDlgIndicationPrimitive {
protected:
  TDialogueId               _dlgId;
  const EncodedOID *        _acOId;
  const TDlgUserInfoList *  _usrInfo;
  const TComponentsList  *  _comps;

public:
  TDlgIndicationPrimitive()
    : _dlgId(0), _acOId(0), _comps(0), _usrInfo(0)
  { }
  virtual ~TDlgIndicationPrimitive()
  { }

  //
  TDialogueId getDialogueId(void) const { return _dlgId; }
  //
  const TDlgUserInfoList * getUserInfo(void) const { return _usrInfo; }
  //
  const EncodedOID * getAppCtx(void) const { return _acOId; }
  //
  const TComponentsList * getCompList(void) const { return _comps; }
};

//
class TC_Begin_Ind : public TDlgIndicationPrimitive {
protected:
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Begin_Ind() : TDlgIndicationPrimitive()
  { }
  ~TC_Begin_Ind()
  { }

  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
};
//
class TC_Cont_Ind : public TDlgIndicationPrimitive {
protected:
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Cont_Ind() : TDlgIndicationPrimitive()
  { }
  ~TC_Cont_Ind()
  { }

  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
};
//
class TC_End_Ind : public TDlgIndicationPrimitive {
public:
  TC_End_Ind() : TDlgIndicationPrimitive()
  { }
  ~TC_End_Ind()
  { }
};
//
class TC_PAbort_Ind : public TDlgIndicationPrimitive {
protected:
  PAbort::Cause_t _cause;

public:
  TC_PAbort_Ind() : TDlgIndicationPrimitive()
    , _cause(PAbort::p_genericError)
  { }
  ~TC_PAbort_Ind()
  { }

  PAbort::Cause_t getCause(void) const { return _cause; }
};
//
class TC_UAbort_Ind : public TDlgIndicationPrimitive {
protected:
  TDialogueAssociate::Diagnostic_t _diagnostic;

public:
  TC_UAbort_Ind() : TDlgIndicationPrimitive()
    , _diagnostic(0)
  { }
  ~TC_UAbort_Ind()
  { }

  TDialogueAssociate::Diagnostic_t getDiagnostic(void) const
  {
    return _diagnostic;
  }
};

//
class TC_Notice_Ind : public TDlgIndicationPrimitive {
protected:
  sccp::ReturnCause_t _cause;
  SCCPAddress         _orgAdr;
  SCCPAddress         _dstAdr;

public:
  TC_Notice_Ind() : TDlgIndicationPrimitive()
    , _cause(sccp::ReturnCause::rcReserved)
  { }
  ~TC_Notice_Ind()
  { }

  sccp::ReturnCause_t getReturnCause(void) const { return _cause; }
  //
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__ */

