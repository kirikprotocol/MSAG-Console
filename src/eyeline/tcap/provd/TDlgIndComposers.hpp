/* ************************************************************************** *
 * TCAP dialogue indication composers.
 * ************************************************************************** */
#ifndef __ELC_TCAP_INDICATIONS_COMPOSERS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_INDICATIONS_COMPOSERS_HPP

#include "eyeline/tcap/TDialogueIndicationPrimitives.hpp"

namespace eyeline {
namespace tcap {
namespace provd {


template <class T_DLG_IND_Arg /* pubic: TDialogueIndicationPrimitive */>
class TDlgIndicationComposerT : public T_DLG_IND_Arg {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac = NULL)
    : T_DLG_IND_Arg(use_ac)
  { }
  //
  void setDialogueId(const TDialogueId & use_id) { _dlgId = use_id; }
  //
  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
};


// -----------------------------------------------
// -- specialization: TC_Begin_Ind composer
// -----------------------------------------------
class TBeginIndComposer : public TDlgIndicationComposerT<TC_Begin_Ind> {
public:
  TBeginIndComposer(const EncodedOID * use_ac = NULL)
    : TDlgIndicationComposerT<TC_Begin_Ind>(use_ac)
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
  void setCompList(ROSComponentsList * use_comps) { _comps = use_comps; }
};

// -----------------------------------------------
// -- specialization: TC_Cont_Ind composer
// -----------------------------------------------
class TContIndComposer : public TDlgIndicationComposerT<TC_Cont_Ind> {
public:
  TContIndComposer(const EncodedOID * use_ac = NULL)
    : TDlgIndicationComposerT<TC_Cont_Ind>(use_ac)
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
  void setCompList(ROSComponentsList * use_comps) { _comps = use_comps; }
};

// -----------------------------------------------
// -- specialization: TC_End_Ind composer
// -----------------------------------------------
class TEndIndComposer : public TDlgIndicationComposerT<TC_End_Ind> {
public:
  TEndIndComposer(const EncodedOID * use_ac = NULL)
    : TDlgIndicationComposerT<TC_End_Ind>(use_ac)
  { }
  //
  void setCompList(ROSComponentsList * use_comps) { _comps = use_comps; }
};

// -----------------------------------------------
// -- specialization: TC_PAbort_Ind composer
// -----------------------------------------------
class TPAbortIndComposer : public TDlgIndicationComposerT<TC_PAbort_Ind> {
public:
  TPAbortIndComposer(const EncodedOID * use_ac = NULL)
    : TDlgIndicationComposerT<TC_PAbort_Ind>(use_ac)
  { }

  void setPAbortCause(PAbort::Cause_e use_cause) { _cause = use_cause; }
};

// -----------------------------------------------
// -- specialization: TC_UAbort_Ind composer
// -----------------------------------------------
class TUAbortIndComposer : public TDlgIndicationComposerT<TC_UAbort_Ind> {
public:
  TUAbortIndComposer(const EncodedOID * use_ac = NULL)
    : TDlgIndicationComposerT<TC_UAbort_Ind>(use_ac)
  { }
  //TODO: setters for abort data
};

// -----------------------------------------------
// -- specialization: TC_Notice_Ind composer
// -----------------------------------------------
class TNoticeIndComposer : public TDlgIndicationComposerT<TC_Notice_Ind> {
public:
  TNoticeIndComposer(const EncodedOID * use_ac = NULL)
    : TDlgIndicationComposerT<TC_Notice_Ind>(use_ac)
  { }

  void setReportCause(ReportCause_e use_cause) { _cause = use_cause; }
  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
  void setCompList(ROSComponentsList * use_comps) { _comps = use_comps; }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_INDICATIONS_COMPOSERS_HPP */

