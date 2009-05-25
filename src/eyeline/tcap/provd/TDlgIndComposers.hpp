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


template <class _TArg /* pubic: TDialogueIndicationPrimitive */>
class TDlgIndicationComposerT : public _TArg {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac = NULL,
                          ROSComponentsList * use_comps = NULL);
  //
  _TArg & TInd(void);
  //
  void setDialogueId(const TDialogueId & use_id);
  //
  void setTransactionId(const TransactionId & use_id);
};


// -----------------------------------------------
// -- specialization: TC_Begin_Ind composer
// -----------------------------------------------
template <>
class TDlgIndicationComposerT<TC_Begin_Ind> : public TC_Begin_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_Begin_Ind(use_ac, use_comps)
  { }

  TC_Begin_Ind & TInd(void) { return *this; }
  //
  void setDialogueId(const TDialogueId & use_id) { _dlgId = use_id; }
  //
  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
};
//
class TBeginIndComposer : public TDlgIndicationComposerT<TC_Begin_Ind> {
public:
  TBeginIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_Begin_Ind>(use_ac, use_comps)
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
};

// -----------------------------------------------
// -- specialization: TC_Cont_Ind composer
// -----------------------------------------------
template <>
class TDlgIndicationComposerT<TC_Cont_Ind> : public TC_Cont_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_Cont_Ind(use_ac, use_comps)
  { }

  TC_Cont_Ind & TInd(void) { return *this; }
  //
  void setDialogueId(const TDialogueId & use_id) { _dlgId = use_id; }
  //
  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
};
//
class TContIndComposer : public TDlgIndicationComposerT<TC_Cont_Ind> {
public:
  TContIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_Cont_Ind>(use_ac, use_comps)
  { }

  void setOrigAddress(const SCCPAddress & use_adr) { _orgAdr = use_adr; }
  void setDestAddress(const SCCPAddress & use_adr) { _dstAdr = use_adr; }
};

// -----------------------------------------------
// -- specialization: TC_End_Ind composer
// -----------------------------------------------
template <>
class TDlgIndicationComposerT<TC_End_Ind> : public TC_End_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_End_Ind(use_ac, use_comps)
  { }

  TC_End_Ind & TInd(void) { return *this; }
  //
  void setDialogueId(const TDialogueId & use_id) { _dlgId = use_id; }
  //
  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
};
//
class TEndIndComposer : public TDlgIndicationComposerT<TC_End_Ind> {
public:
  TEndIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_End_Ind>(use_ac, use_comps)
  { }
};

// -----------------------------------------------
// -- specialization: TC_PAbort_Ind composer
// -----------------------------------------------
template <>
class TDlgIndicationComposerT<TC_PAbort_Ind> : public TC_PAbort_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_PAbort_Ind(use_ac)
  { }

  TC_PAbort_Ind & TInd(void) { return *this; }
  //
  void setDialogueId(const TDialogueId & use_id) { _dlgId = use_id; }
  //
  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
};
//
class TPAbortIndComposer : public TDlgIndicationComposerT<TC_PAbort_Ind> {
public:
  TPAbortIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_PAbort_Ind>(use_ac, use_comps)
  { }

  void setPAbortCause(PAbort::Cause_e use_cause) { _cause = use_cause; }
};

// -----------------------------------------------
// -- specialization: TC_UAbort_Ind composer
// -----------------------------------------------
template <>
class TDlgIndicationComposerT<TC_UAbort_Ind> : public TC_UAbort_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_UAbort_Ind(use_ac)
  { }

  TC_UAbort_Ind & TInd(void) { return *this; }
  //
  void setDialogueId(const TDialogueId & use_id) { _dlgId = use_id; }
  //
  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
};
//
class TUAbortIndComposer : public TDlgIndicationComposerT<TC_UAbort_Ind> {
public:
  TUAbortIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_UAbort_Ind>(use_ac, use_comps)
  { }
};

// -----------------------------------------------
// -- specialization: TC_Notice_Ind composer
// -----------------------------------------------
template <>
class TDlgIndicationComposerT<TC_Notice_Ind> : public TC_Notice_Ind {
public:
  TDlgIndicationComposerT(const EncodedOID * use_ac, ROSComponentsList * use_comps)
    : TC_Notice_Ind(use_ac, use_comps)
  { }

  TC_Notice_Ind & TInd(void) { return *this; }
  //
  void setDialogueId(const TDialogueId & use_id) { _dlgId = use_id; }
  //
  void setTransactionId(const TransactionId & use_id) { _trId = use_id; }
};
//
class TNoticeIndComposer : public TDlgIndicationComposerT<TC_Notice_Ind> {
public:
  TNoticeIndComposer(const EncodedOID * use_ac = NULL,
                    ROSComponentsList * use_comps = NULL)
    : TDlgIndicationComposerT<TC_Notice_Ind>(use_ac, use_comps)
  { }

  void setReportCause(ReportCause_e use_cause) { _cause = use_cause; }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_INDICATIONS_COMPOSERS_HPP */

