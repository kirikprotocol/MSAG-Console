/* ************************************************************************** *
 * TCAP API: structured TCAP dialogue requests primitives(transaction sublayer).
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__

#include "eyeline/sccp/SCCPAddress.hpp"
#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/tcap/TDlgUserInfo.hpp"
#include "eyeline/tcap/TComponentDefs.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;
using eyeline::asn1::ASExternal;
using eyeline::ros::ROSPdu;
using eyeline::sccp::SCCPAddress;

class TDlgRequestPrimitive {
public:
  enum RKind_e {
    reqTRNone = 0, reqTRBegin, reqTRCont, reqTREnd, reqTRUAbort, reqTRPAbort
  };

  virtual ~TDlgRequestPrimitive()
  { }

  static const char * getIdent(RKind_e req_kind);

  const char * getIdent(void) const { return getIdent(_rKind); }

  //
  void setDialogueId(const TDialogueId & use_id) { _dlgId = use_id; }
  const TDialogueId & getDialogueId(void) const { return _dlgId; }

  //NOTE: overrides appCtx from TC_Req
  void setAppCtx(const EncodedOID & use_acid) { _acOId = &use_acid; }
  const EncodedOID * getAppCtx(void) const { return _acOId; }
  //
  void setReturnOnError(bool ret_on_error = true) { _retOnErr = ret_on_error; }
  bool getReturnOnError(void) const { return _retOnErr; }
  //
  void setInSequenceDelivery(bool use_seq_dlvr = true) { _inSeqDelivery = use_seq_dlvr; }
  bool getInSequenceDelivery(void) const { return _inSeqDelivery; }
  //
  void addUIValue(const ASExternal & use_ui) { _usrInfo.push_back(&use_ui); }
  const TDlgUserInfoPtrList & getUserInfo(void) const { return _usrInfo; }

protected:
  const RKind_e _rKind;
  //Next two parameters composes TCAP dialogue 'quality of service' parameter
  bool          _retOnErr;
  bool          _inSeqDelivery;
  // -- Transaction portion parameteres
  TDialogueId   _dlgId;
  // -- Dialogue portion parameteres
  const EncodedOID *  _acOId;
  TDlgUserInfoPtrList _usrInfo;
  // -- Component portion parameteres
  TComponentsPtrList  _comps;


  TDlgRequestPrimitive(RKind_e req_kind)
    : _rKind(req_kind), _retOnErr(false), _inSeqDelivery(true), _acOId(0)
  { }
};

//
class TR_Begin_Req : public TDlgRequestPrimitive {
protected:
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TR_Begin_Req() : TDlgRequestPrimitive(reqTRBegin)
  { }
  ~TR_Begin_Req()
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  //
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
  //
  void addTComponent(const ROSPdu & use_comp) { _comps.push_back(&use_comp); }
  const TComponentsPtrList & getCompList(void) const { return _comps; }
};

//
class TR_Cont_Req : public TDlgRequestPrimitive {
protected:
  SCCPAddress   _orgAdr;

public:
  TR_Cont_Req() : TDlgRequestPrimitive(reqTRCont)
  { }
  ~TR_Cont_Req()
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  //
  void addTComponent(const ROSPdu & use_comp) { _comps.push_back(&use_comp); }
  const TComponentsPtrList & getCompList(void) const { return _comps; }
};

//
class TR_End_Req : public TDlgRequestPrimitive {
protected:
  SCCPAddress   _orgAdr;

public:
  enum DialogEnding_e { endBASIC = 0, endPREARRANGED };

  TR_End_Req(DialogEnding_e use_term = endBASIC)
    : TDlgRequestPrimitive(reqTREnd), _termination(use_term)
  { }
  ~TR_End_Req()
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  //
  void setPrearrangedEnd(void) { _termination = endPREARRANGED; }
  DialogEnding_e getTermination() const { return _termination; }
  //
  void addTComponent(const ROSPdu & use_comp) { _comps.push_back(&use_comp); }
  const TComponentsPtrList & getCompList(void) const { return _comps; }

protected:
  DialogEnding_e        _termination;
};

class TR_UAbort_Req : public TDlgRequestPrimitive {
public:
  enum Kind_e {         //There are 3 kinds of user abort:
    uabrtAssociation = 0  //TCUser aborts establishing a dialog
    , uabrtDialogueUI     //TCUser aborts already established dialog with optional
                          //UserInfo provided as reason.
    , uabrtDialogueEXT    //TCUser aborts already established dialog with some
                          //externally defined DataValue provided as reason.
  };

  TR_UAbort_Req() : TDlgRequestPrimitive(reqTRUAbort)
    , _ascRejCause(TDialogueAssociate::dsu_null)
  { }
  ~TR_UAbort_Req()
  { }

  const Kind_e & getAbortKind(void) const { return _kind; }

  //abort of dialogue establishing with optional UserInfo provided.
  //NOTE: add more UserInfo values by addUIValue() calls next to this one!!!
  //NOTE: This method only for 1st response to T_Begin_Ind
  void abortAssociation(TDialogueAssociate::DiagnosticUser_e
                          use_cause = TDialogueAssociate::dsu_null,
                        const ASExternal * use_ext = NULL)
  {
    _kind = uabrtAssociation;
    _ascRejCause = use_cause;
    if (use_ext)
      addUIValue(*use_ext);
  }

  //abort of established dialogue with optional UserInfo provided as reason
  //NOTE: add more UserInfo values by addUIValue() calls next to this one!!!
  void abortDialogueUI(const ASExternal * use_ext = NULL)
  {
    _kind = uabrtDialogueUI;
    _usrInfo.clear();
    if (use_ext)
      addUIValue(*use_ext);
  }
  //abort of established dialogue with some externally defined DataValue provided as reason
  void abortDialogueEXT(const ASExternal & use_ext)
  {
    _kind = uabrtDialogueEXT;
    _usrInfo.clear();
    addUIValue(use_ext);
  }
  //
  TDialogueAssociate::DiagnosticUser_e getAssociateDiagnostic(void) const
  {
    return _ascRejCause;
  }

protected:
  Kind_e  _kind;
  TDialogueAssociate::DiagnosticUser_e _ascRejCause; //associate reject diagnostic
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__ */

