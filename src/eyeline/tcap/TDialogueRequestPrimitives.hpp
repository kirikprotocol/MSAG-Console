/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue requests.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__

# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/ros/ROSCompList.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/TDialogueDefs.hpp"
# include "eyeline/tcap/TDlgUserInfo.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;
using eyeline::ros::ROSComponentsList;
using eyeline::ros::ROSComponentRfp;
using eyeline::sccp::SCCPAddress;

class TDialogueRequestPrimitive {
protected:
  bool          _retOnErr;
  TDialogueId   _dlgId;
  TDlgUserInfo  _usrInfo;

  ROSComponentsList   _comps;
  const EncodedOID *  _acOId;

public:
  TDialogueRequestPrimitive()
    : _retOnErr(false), _acOId(0)
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
  void setReturnOnError() { _retOnErr = true; }
  bool getReturnOnError() const { return _retOnErr; }

  //
  void addUIValue(const UIValue & use_ui) { _usrInfo.addUIValue(use_ui); }
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
  void addROSComponent(const ROSComponentRfp & use_comp) { _comps.push_back(use_comp); }
  const ROSComponentsList & CompList(void) const { return _comps; }
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
  void addROSComponent(const ROSComponentRfp & use_comp) { _comps.push_back(use_comp); }
  const ROSComponentsList & CompList(void) const { return _comps; }
};

//
class TC_End_Req : public TDialogueRequestPrimitive {
public:
  enum DialogEnding_e { endBASIC = 0, endPREARRANGED };

protected:
  DialogEnding_e        _termination;

public:
  TC_End_Req()
    : _termination(endBASIC)
  { }

  void setPrearrangedEnd(void) { _termination = endPREARRANGED; }
  DialogEnding_e getTermination() const { return _termination; }
  //
  void addROSComponent(const ROSComponentRfp & use_comp) { _comps.push_back(use_comp); }
  const ROSComponentsList & CompList(void) const { return _comps; }
};

//
class TC_UAbort_Req : public TDialogueRequestPrimitive {
protected:
  TCUserAssociateResult  _ascResultUsr;

public:
  TC_UAbort_Req()
    : _ascResultUsr(false)
  { }

  const TCUserAssociateResult & userAssociateResult(void) const { return _ascResultUsr; }
  //NOTE: These methods only for 1st response to T_Begin_Ind
  void rejectDlgByUser(TDialogueAssociate::DiagnosticUser_e use_cause =
                      TDialogueAssociate::dsu_null)
  {
//    _ascResultUsr._accepted = false;
    _ascResultUsr._rejCause = use_cause;
  }
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_PROVD_TDLGREQUESTPRIMITIVES_HPP__ */

