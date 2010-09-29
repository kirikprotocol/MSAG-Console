#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/TDlgIndComposers.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

/* ************************************************************************* *
 * class TBeginIndComposer implementation
 * ************************************************************************* */
void TBeginIndComposer::init(proto::TMsgBegin & use_msg) /*throw(std::exception)*/
{
  _orgTrId = use_msg._orgTrId;
  //link OID, CompPart, UserInfo
  if (use_msg._dlgPart.empty()) {
    //NOTE: contextless TC primitives MUST have a component with opcode defined !!!
    if (use_msg._compPart.empty())
      throw smsc::util::Exception("tcap::provd::TBeginIndComposer : neither appContext nor component is defined");
    _acOId = &_ac_contextless_ops;
  } else {
    _acOId = &(use_msg.getAARQ()->_acId);
    if (!use_msg.getAARQ()->_usrInfo.empty())
      _usrInfo = &use_msg.getAARQ()->_usrInfo;
  }
  if (!use_msg._compPart.empty()) {
    _comps = &use_msg._compPart;
    //only ROSInvoke PDUs are allowed in TR-Begin
    for (TComponentsList::const_iterator it = _comps->begin(); it != _comps->end(); ++it) {
      const ros::ROSPduPrimitiveAC * rosComp = it->get();
      if (!rosComp->isInvoke())
        throw smsc::util::Exception("tcap::provd::TBeginIndComposer : not an Invoke component detected");
    }
  }
}

/* ************************************************************************* *
 * class TContIndComposer implementation
 * ************************************************************************* */
void TContIndComposer::init(proto::TMsgContinue & use_msg) /*throw(std::exception)*/
{
  _orgTrId = use_msg._orgTrId;
  _dstTrId = use_msg._dstTrId;
  //link OID, CompPart, UserInfo
  if (!use_msg._dlgPart.empty()) {
    _acOId = &(use_msg.getAARE()->_acId);
    if (!use_msg.getAARE()->_usrInfo.empty())
      _usrInfo = &use_msg.getAARE()->_usrInfo;
  }
  if (!use_msg._compPart.empty())
    _comps = &use_msg._compPart;
}

/* ************************************************************************* *
 * class TEndIndComposer implementation
 * ************************************************************************* */
void TEndIndComposer::init(proto::TMsgEnd & use_msg) /*throw(std::exception)*/
{
  _dstTrId = use_msg._dstTrId;
  //link OID, CompPart, UserInfo
  if (!use_msg._dlgPart.empty()) {
    _acOId = &(use_msg.getAARE()->_acId);
    if (!use_msg.getAARE()->_usrInfo.empty())
      _usrInfo = &use_msg.getAARE()->_usrInfo;
  }
  if (!use_msg._compPart.empty())
    _comps = &use_msg._compPart;
}

/* ************************************************************************* *
 * class TUAbortIndComposer implementation
 * ************************************************************************* */
void TUAbortIndComposer::init(proto::TMsgAbort & use_msg) /*throw(std::exception)*/
{
  _dstTrId = use_msg._dstTrId;
  if (use_msg.getABRT()) {  //abort of already associated dialogue
    if (!use_msg.getABRT()->_usrInfo.empty())
      _usrInfo = &use_msg.getABRT()->_usrInfo;
  } else if (use_msg.getAARE()) { //abort of dialogue association request
    _diagnostic = use_msg.getAARE()->_diagnostic._sdResult;
    if (!use_msg.getAARE()->_usrInfo.empty())
      _usrInfo = &use_msg.getAARE()->_usrInfo;
  } else if (use_msg.getEXT()) { //abort of already associated dialogue
      _usrInfo = use_msg.getEXT();
  }
}
   
} //provd
} //tcap
} //eyeline

