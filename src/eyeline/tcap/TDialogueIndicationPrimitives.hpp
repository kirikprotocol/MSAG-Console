/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue indications.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__

# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/ros/ROSCompList.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TDialogueDefs.hpp"
# include "eyeline/tcap/TDlgUserInfo.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;
using eyeline::ros::ROSComponentsList;
using eyeline::sccp::SCCPAddress;
using eyeline::tcap::proto::TransactionId;
using eyeline::tcap::PAbort;

class TDialogueIndicationPrimitive {
protected:
  TDialogueId   _dlgId;
  TransactionId _trId;
  TDlgUserInfo  _usrInfo;

  const EncodedOID *  _acOId;
  ROSComponentsList * _comps;

public:
  TDialogueIndicationPrimitive(const EncodedOID * use_ac = NULL)
    : _acOId(use_ac), _comps(NULL)
  { }
  virtual ~TDialogueIndicationPrimitive()
  { }

  //
  TDialogueId getDialogueId(void) const { return _dlgId; }
  //
  const TransactionId & getTransactionId(void) const  { return _trId; }
  //
  TDlgUserInfo & getUserInfo(void) { return _usrInfo; }
  //
  const EncodedOID * getAppCtx(void) const { return _acOId; }
  //
  ROSComponentsList * CompList(void) { return _comps; }
};

//
class TC_Begin_Ind : public TDialogueIndicationPrimitive {
protected:
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Begin_Ind(const EncodedOID * use_ac = NULL)
    : TDialogueIndicationPrimitive(use_ac)
  { }

  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
  //TODO: getters
};
//
class TC_Cont_Ind : public TDialogueIndicationPrimitive {
protected:
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Cont_Ind(const EncodedOID * use_ac = NULL)
    : TDialogueIndicationPrimitive(use_ac)
  { }

  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
  //TODO: getters
};
//
class TC_End_Ind : public TDialogueIndicationPrimitive {
public:
  TC_End_Ind(const EncodedOID * use_ac = NULL)
    : TDialogueIndicationPrimitive(use_ac)
  { }

  //TODO: getters
};
//
class TC_UAbort_Ind : public TDialogueIndicationPrimitive {
public:
  TC_UAbort_Ind(const EncodedOID * use_ac = NULL)
    : TDialogueIndicationPrimitive(use_ac)
  { }

  //TODO: getters for TCAbrtPDU or TCExternal
};
//
class TC_PAbort_Ind : public TDialogueIndicationPrimitive {
protected:
  PAbort::Cause_e _cause;

public:
  TC_PAbort_Ind(const EncodedOID * use_ac = NULL)
    : TDialogueIndicationPrimitive(use_ac)
    , _cause(PAbort::p_genericError)
  { }

  PAbort::Cause_e getPAbortCause(void) const { return _cause; }
};

//
class TC_Notice_Ind : public TDialogueIndicationPrimitive {
public:
  enum ReportCause_e {
    errOk = 0
    //SCCP/network related causes
    , errNoAdrTranslation
    , errSSNCongest
    , errSSNFailure
    , errUserUneqipped
    , errNetworkConjestion
    , errNetworkFailure
    , errHOPCounter
    , errSCCPFailure          //unqualified SCCP error
    //TCAP related causes
    , errDialoguePortion      //bad dialogue PDU data, usually a UserInfo
    , errTCAPFailure          //unqualified TCAP error
  };

protected:
  ReportCause_e _cause;
  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Notice_Ind(const EncodedOID * use_ac = NULL)
    : TDialogueIndicationPrimitive(use_ac)
    , _cause(errOk)
  { }

  ReportCause_e getReportCause(void) const { return _cause; }
  //
  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__ */

