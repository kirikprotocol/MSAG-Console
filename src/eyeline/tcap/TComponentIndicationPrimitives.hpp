/* ************************************************************************** *
 * TComponent sublayer: component handling indications.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__

# include "eyeline/tcap/TDialogueDefs.hpp"
# include "eyeline/tcap/TComponentDefs.hpp"

namespace eyeline {
namespace tcap {

using eyeline::ros::RejectProblem;

class TComponentIndicationPrimitive {
protected:
  TDialogueId   _dlgId;
  InvokeId      _invokeId;
  bool          _hasInvId;

public:
  TComponentIndicationPrimitive()
    : _dlgId(0), _invokeId(0), _hasInvId(false)
  { }
  ~TComponentIndicationPrimitive()
  { }
  //
  TDialogueId getDialogueId(void) const { return _dlgId; }
  //
  bool hasInvokeId(void) const { return _hasInvId; }
  //
  InvokeId getInvokeId(void) const { return _invokeId; }
};


//NOTE: InvokeId is optional!
class TC_L_Cancel_Ind : public TComponentIndicationPrimitive { 
public:
  TC_L_Cancel_Ind()
  { }
  ~TC_L_Cancel_Ind()
  { }
};

//NOTE: InvokeId is optional!
class TC_L_Reject_Ind : public TComponentIndicationPrimitive {
protected:
  RejectProblem   _rejProblem;

public:
  TC_L_Reject_Ind()
  { }
  ~TC_L_Reject_Ind()
  { }

  const RejectProblem & getProblem(void) const { return _rejProblem; }
};

class TC_R_Reject_Ind : public TComponentIndicationPrimitive {
protected:
  RejectProblem   _rejProblem;

public:
  TC_R_Reject_Ind()
  { }
  ~TC_R_Reject_Ind()
  { }

  const RejectProblem & getProblem(void) const { return _rejProblem; }
};

//NOTE: InvokeId is mandatory!
class TC_U_Reject_Ind : public TComponentIndicationPrimitive {
protected:
  RejectProblem   _rejProblem;

public:
  TC_U_Reject_Ind()
  { }
  ~TC_U_Reject_Ind()
  { }

  const RejectProblem & getProblem(void) const { return _rejProblem; }
};

}}

#endif /* __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__ */

