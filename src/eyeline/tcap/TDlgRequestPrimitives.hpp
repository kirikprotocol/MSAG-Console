/* ************************************************************************** *
 * TCAP API: structured TCAP dialogue requests primitives.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__

# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/TDialogueDefs.hpp"
# include "eyeline/tcap/TDlgUserInfo.hpp"
# include "eyeline/tcap/TComponentDefs.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;
using eyeline::asn1::ASExternal;
using eyeline::ros::ROSPduPrimitiveAC;
using eyeline::sccp::SCCPAddress;

class TDlgRequestPrimitive {
protected:
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

public:
  TDlgRequestPrimitive()
    : _retOnErr(false), _inSeqDelivery(false), _acOId(0)
  { }
  virtual ~TDlgRequestPrimitive()
  { }

  //
  void setDialogueId(const TDialogueId & use_id) { _dlgId = use_id; }
  const TDialogueId & getDialogueId(void) const { return _dlgId; }
  //
  void setAppCtx(const EncodedOID & use_acid) { _acOId = &use_acid; }
  const EncodedOID * getAppCtx(void) const { return _acOId; }
  //
  void setReturnOnError(bool ret_on_error = true) { _retOnErr = ret_on_error; }
  bool getReturnOnError(void) const { return _retOnErr; }
  //
  void setInSequenceDelivery(void) { _inSeqDelivery = true; }
  bool getInSequenceDelivery(void) const { return _inSeqDelivery; }
  //
  void addUIValue(const ASExternal & use_ui) { _usrInfo.push_back(&use_ui); }
  const TDlgUserInfoPtrList & getUserInfo(void) const { return _usrInfo; }
};

//
class TC_Begin_Req : public TDlgRequestPrimitive {
protected:
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Begin_Req()
  { }
  ~TC_Begin_Req()
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  //
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
  //
  void addTComponent(const ROSPduPrimitiveAC & use_comp) { _comps.push_back(&use_comp); }
  const TComponentsPtrList & getCompList(void) const { return _comps; }
};

//
class TC_Cont_Req : public TDlgRequestPrimitive {
protected:
  SCCPAddress   _orgAdr;

public:
  TC_Cont_Req()
  { }
  ~TC_Cont_Req()
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  //
  void addTComponent(const ROSPduPrimitiveAC & use_comp) { _comps.push_back(&use_comp); }
  const TComponentsPtrList & getCompList(void) const { return _comps; }
};

//
class TC_End_Req : public TDlgRequestPrimitive {
public:
  enum DialogEnding_e { endBASIC = 0, endPREARRANGED };

protected:
  DialogEnding_e        _termination;

public:
  TC_End_Req(DialogEnding_e use_term = endBASIC)
    : _termination(use_term)
  { }
  ~TC_End_Req()
  { }

  void setPrearrangedEnd(void) { _termination = endPREARRANGED; }
  DialogEnding_e getTermination() const { return _termination; }
  //
  void addTComponent(const ROSPduPrimitiveAC & use_comp) { _comps.push_back(&use_comp); }
  const TComponentsPtrList & getCompList(void) const { return _comps; }
};


class TC_UAbort_Req : public TDlgRequestPrimitive {
public:
  enum Kind_e {         //There are 3 kinds of user abort:
    uabrtAssociation = 0  //TCUser aborts establishing a dialog
    , uabrtDialogueUI     //TCUser aborts already established dialog with optional
                          //UserInfo provided as reason.
    , uabrtDialogueEXT    //TCUser aborts already established dialog with some
                          //externally defined DataValue provided as reason.
  };

protected:
  Kind_e  _kind;
  bool    _abortFromTCUser; //TCProvider able to generate UAbort in case of unsupported AppCtx
  TDialogueAssociate::DiagnosticUser_e _ascRejCause; //associate reject diagnostic

public:
  TC_UAbort_Req(bool abort_from_tc_user = true)
    : _kind(uabrtDialogueUI), _abortFromTCUser(abort_from_tc_user)
    , _ascRejCause(TDialogueAssociate::dsu_null)
  { }
  ~TC_UAbort_Req()
  { }

  bool isAbortFromTCUser() const { return _abortFromTCUser; }

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
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__ */

