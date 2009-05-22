/* ************************************************************************** *
 * Classes implementing structured TCAP dialogue indications.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__

# include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"

namespace eyeline {
namespace tcap {

using eyeline::sccp::SCCPAddress;

class TDialogueIndicationPrimitive : public TDialogueHandlingPrimitive {
protected:
  const EncodedOID *  _acOId;
  ROSComponentsList * _comps;
  TDlgUserInfo *      _usrInfo;

public:
  TDialogueIndicationPrimitive(const EncodedOID * use_ac = NULL,
                               ROSComponentsList * use_comps = NULL,
                               TDlgUserInfo * use_ui = NULL)
    : _acOId(use_ac), _comps(use_comps), _usrInfo(use_ui)
  { }
  virtual ~TDialogueIndicationPrimitive()
  { }

  // ------------------------------------------------
  // -- TDialogueHandlingPrimitive interface methods
  // ------------------------------------------------
  virtual const EncodedOID * getAppCtx(void) const { return _acOId; }
  virtual ROSComponentsList * CompList(void) { return _comps; }
  virtual TDlgUserInfo * UserInfo(void) { return _usrInfo; }

  //TODO: indications specific stuff
};

//
class TC_Begin_Ind : public TDialogueIndicationPrimitive {
protected:
  //TODO:  friend eyeline::tcap::provd::TBeginIndDispatcher;

  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Begin_Ind(const EncodedOID * use_ac = NULL,
               ROSComponentsList * use_comps = NULL, TDlgUserInfo * use_ui = NULL)
    : TDialogueIndicationPrimitive(use_ac, use_comps, use_ui)
  { }

  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
  //TODO: getters
};
//
class TC_Cont_Ind : public TDialogueIndicationPrimitive {
protected:
  //TODO: friend eyeline::tcap::provd::TContIndDispatcher;

  SCCPAddress   _orgAdr;
  SCCPAddress   _dstAdr;

public:
  TC_Cont_Ind(const EncodedOID * use_ac = NULL,
               ROSComponentsList * use_comps = NULL, TDlgUserInfo * use_ui = NULL)
    : TDialogueIndicationPrimitive(use_ac, use_comps, use_ui)
  { }

  const SCCPAddress & getOrigAddress(void) const { return _orgAdr; }
  const SCCPAddress & getDestAddress(void) const { return _dstAdr; }
  //TODO: getters
};
//
class TC_End_Ind : public TDialogueIndicationPrimitive {
public:
  TC_End_Ind(const EncodedOID * use_ac = NULL,
               ROSComponentsList * use_comps = NULL, TDlgUserInfo * use_ui = NULL)
    : TDialogueIndicationPrimitive(use_ac, use_comps, use_ui)
  { }

  //TODO: getters
};
//
class TC_UAbort_Ind : public TDialogueIndicationPrimitive {
public:
  TC_UAbort_Ind(const EncodedOID * use_ac = NULL,
                TDlgUserInfo * use_ui = NULL)
    : TDialogueIndicationPrimitive(use_ac, 0, use_ui)
  { }

  //TODO: getters for TCAbrtPDU or TCExternal
};
//
class TC_PAbort_Ind : public TDialogueIndicationPrimitive {
protected:
  PAbortCause_e _cause;

public:
  TC_PAbort_Ind(const EncodedOID * use_ac = NULL)
    : TDialogueIndicationPrimitive(use_ac, 0, 0)
    , _cause(TDialogueHandlingPrimitive::p_genericError)
  { }

  void setPAbortCause(PAbortCause_e use_cause) { _cause = use_cause; }
  PAbortCause_e getPAbortCause(void) const { return _cause; }
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

public:
  TC_Notice_Ind(const EncodedOID * use_ac = NULL,
               ROSComponentsList * use_comps = NULL, TDlgUserInfo * use_ui = NULL)
    : TDialogueIndicationPrimitive(use_ac, use_comps, use_ui)
    , _cause(INVALID_TCUSER_DATA)
  { }

  void setReportCause(ReportCause_e use_cause) { _cause = use_cause; }
  ReportCause_e getReportCause(void) const { return _cause; }
};

} //tcap
} //eyeline

#endif /* __EYELINE_TCAP_TDIALOGUEINDICATIONPRIMITIVE_HPP__ */

