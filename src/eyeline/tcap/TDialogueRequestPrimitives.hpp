/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue requests.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__

# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/ros/ROSCompList.hpp"
# include "eyeline/ros/ROSComponent.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/TDialogueDefs.hpp"
# include "eyeline/tcap/TDlgUserInfo.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;
using eyeline::asn1::ASExternalValue;

using eyeline::ros::ROSComponentsList;

using eyeline::sccp::SCCPAddress;


class TDialogueRequestPrimitive {
protected:
  //Next two parameters composes TCAP dialogue 'quality of service' parameter
  bool          _retOnErr;
  bool          _inSeqDelivery;
  //
  TDialogueId   _dlgId;
  TDlgUserInfo  _usrInfo;

  ROSComponentsList   _comps;
  const EncodedOID *  _acOId;

public:
  TDialogueRequestPrimitive()
    : _retOnErr(false), _inSeqDelivery(false), _acOId(0)
  { }
  virtual ~TDialogueRequestPrimitive()
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
  void addUIValue(const ASExternalValue & use_ui) { _usrInfo.addUIValue(use_ui); }
  const TDlgUserInfo * getUserInfo(void) const { return _usrInfo.empty() ? 0 : &_usrInfo; }
};

//
class TC_Begin_Req : public TDialogueRequestPrimitive {
protected:
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Begin_Req()
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  //
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
  //
  void addROSComponent(ros::ROSComponentPrimitive* use_comp) { _comps.push_back(use_comp); }
  const ROSComponentsList & getCompList(void) const { return _comps; }
};

//
class TC_Cont_Req : public TDialogueRequestPrimitive {
protected:
  SCCPAddress   _orgAdr;

public:
  TC_Cont_Req()
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  //
  void addROSComponent(ros::ROSComponentPrimitive* use_comp) { _comps.push_back(use_comp); }
  const ROSComponentsList & getCompList(void) const { return _comps; }
};

//
class TC_End_Req : public TDialogueRequestPrimitive {
public:
  enum DialogEnding_e { endBASIC = 0, endPREARRANGED };

protected:
  DialogEnding_e        _termination;

public:
  TC_End_Req(DialogEnding_e use_term = endBASIC)
    : _termination(use_term)
  { }

  void setPrearrangedEnd(void) { _termination = endPREARRANGED; }
  DialogEnding_e getTermination() const { return _termination; }
  //
  void addROSComponent(ros::ROSComponentPrimitive* use_comp) { _comps.push_back(use_comp); }
  const ROSComponentsList & getCompList(void) const { return _comps; }
};

//There are 3 kinds of user abort:
// 1) abort of dialogue association (in response to T_Begin) with associate
//    diagnostic provided as reason
// 2) abort of established dialogue with optional UserInfo provided as reason
// 3) abort of established dialogue with some externally defined PDU provided
//    as reason
class TC_UAbort_Req : public TDialogueRequestPrimitive {
public:
  enum Kind_e {
    uabrtAssociation = 0, uabrtDialogueUI, uabrtDialogueEXT
  };

protected:
  Kind_e  _kind;
  TDialogueAssociate::DiagnosticUser_e _ascRejCause; //associate reject diagnostic
  bool _abortFromTCUser;
public:
  TC_UAbort_Req(bool abort_from_tc_user=true)
    : _kind(uabrtDialogueUI), _ascRejCause(TDialogueAssociate::dsu_null),
      _abortFromTCUser(abort_from_tc_user)
  {}

  const Kind_e & getAbortKind(void) const { return _kind; }

  //NOTE: This method only for 1st response to T_Begin_Ind
  void abortAssociation(TDialogueAssociate::DiagnosticUser_e use_cause =
                        TDialogueAssociate::dsu_null)
  {
    _kind = uabrtAssociation;
    _ascRejCause = use_cause;
  }

  //abort of established dialogue with optional UserInfo provided as reason
  //NOTE: add UserInfo values by addUIValue() calls next to this one!!!
  void abortDialogue(void)
  {
    _kind = uabrtDialogueUI;
//    _usrInfo.clear();
  }

  //abort of established dialogue with some externally defined PDU provided as reason
  void abortDialogueAS(const ASExternalValue & use_ext)
  {
    _kind = uabrtDialogueEXT;
    _usrInfo.clear();
    _usrInfo.addUIValue(use_ext);
  }
  TDialogueAssociate::DiagnosticUser_e getRejectCause() const { return _ascRejCause; }
  bool isAbortFromTCUser() const { return _abortFromTCUser; }
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__ */

