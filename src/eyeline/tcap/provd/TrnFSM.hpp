#ifndef __EYELINE_TCAP_PROVD_TRNFSM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TRNFSM_HPP__

# include "core/synchronization/Mutex.hpp"

# include "eyeline/tcap/proto/TransactionId.hpp"
# include "eyeline/tcap/TDialogueRequestPrimitives.hpp"
# include "eyeline/tcap/TDialogueIndicationPrimitives.hpp"
# include "eyeline/tcap/provd/TDlgReqSerializer.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TrnFSM {
public:
  explicit TrnFSM(uint32_t localTrnIdValue);
  TrnFSM(uint32_t localTrnIdValue, uint32_t remoteTrnIdValue);

  void updateTransaction(const TC_Begin_Req& dialogueRequestPrimitive);
  void updateTransaction(const TC_Cont_Req& dialogueRequestPrimitive);
  void updateTransaction(const TC_End_Req& dialogueRequestPrimitive);
  void updateTransaction(const TC_UAbort_Req& dialogueRequestPrimitive);

  void updateTransaction(const TC_PAbort_Req& dialogueRequestPrimitive);

  void updateTransaction(const TC_Begin_Ind& dialogueIndicationPrimitive);
  void updateTransaction(const TC_Cont_Ind& dialogueIndicationPrimitive);
  void updateTransaction(const TC_End_Ind& dialogueIndicationPrimitive);
  void updateTransaction(const TC_UAbort_Ind& dialogueIndicationPrimitive);
  void updateTransaction(const TC_PAbort_Ind& dialogueIndicationPrimitive);
  void updateTransaction(const TC_Notice_Ind& dialogueIndicationPrimitive);

  const proto::TransactionId& getTransactionId() const;
  void composeTrnId(const proto::TransactionId& remoteTrnId);
private:
  proto::TransactionId _trnId;
  typedef enum { IDLE, INIT_SENT, INIT_RECEIVED, ACTIVE } transaction_state_t;
  transaction_state_t _transactionState;

  smsc::core::synchronization::Mutex _transactionStateLock;

  const char* const stateToString(transaction_state_t transactionState) const;
};

}}}

#endif
