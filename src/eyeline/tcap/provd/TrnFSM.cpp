#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "core/synchronization/MutexGuard.hpp"
#include "eyeline/utilx/Exception.hpp"

#include "TrnFSM.hpp"
#include "TDialogueServiceData.hpp"

namespace eyeline {
namespace tcap {
namespace provd {


TrnFSM::TrnFSM(TDialogueServiceData* tDlgSvcData, uint32_t localTrnIdValue)
  : _tDlgSvcData(tDlgSvcData), _trnId(proto::TransactionId::orgLocal, localTrnIdValue),
    _transactionState(IDLE)
{}

TrnFSM::TrnFSM(TDialogueServiceData* tDlgSvcData, uint32_t localTrnIdValue, uint32_t remoteTrnIdValue)
  : _tDlgSvcData(tDlgSvcData), _trnId(proto::TransactionId::orgRemote, localTrnIdValue, remoteTrnIdValue),
    _transactionState(IDLE)
{}

void
TrnFSM::updateTransaction(const TC_Begin_Req& dialogueRequestPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( _transactionState != IDLE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[%s value=(%d)] for TC_Begin_Req primitive", stateToString(_transactionState), _transactionState);
  _transactionState = INIT_SENT;
}

void
TrnFSM::updateTransaction(const TC_Cont_Req& dialogueRequestPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( _transactionState != INIT_RECEIVED && _transactionState != ACTIVE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[%s value=(%d)] for TC_Cont_Req primitive", stateToString(_transactionState), _transactionState);

  if ( _transactionState == INIT_RECEIVED )
    _tDlgSvcData->setSrcAddr(dialogueRequestPrimitive.getOrigAddress());

  _transactionState = ACTIVE;
}

void
TrnFSM::updateTransaction(const TC_End_Req& dialogueRequestPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( dialogueRequestPrimitive.getTermination() == TC_End_Req::endPREARRANGED &&  _transactionState == IDLE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[IDLE] for TC_End_Req primitive in prearranged end of transaction");
  else if ( _transactionState != INIT_RECEIVED && _transactionState != ACTIVE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[%s value=(%d)] for TC_End_Req primitive");

  _transactionState = IDLE;
}

void
TrnFSM::updateTransaction(const TC_UAbort_Req& dialogueRequestPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( _transactionState == IDLE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[IDLE] for TC_UAbort_Req primitive");
  _transactionState = IDLE;
}

void
TrnFSM::updateTransaction(const TC_PAbort_Req& dialogueRequestPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  _transactionState = IDLE;
}

void
TrnFSM::updateTransaction(const TC_Begin_Ind& dialogueIndicationPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( _transactionState != IDLE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[%s value=(%d)] for TC_Begin_Ind primitive", stateToString(_transactionState), _transactionState);
  _transactionState = INIT_RECEIVED;
}

void
TrnFSM::updateTransaction(const TC_Cont_Ind& dialogueIndicationPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( _transactionState != INIT_SENT && _transactionState != ACTIVE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[%s value=(%d)] for TC_Cont_Ind primitive", stateToString(_transactionState), _transactionState);
  if ( _transactionState == INIT_SENT ) {
    composeTrnId(dialogueIndicationPrimitive.getTransactionId());
    _tDlgSvcData->setDstAddr(dialogueIndicationPrimitive.getOrigAddress());
  }
  _transactionState = ACTIVE;
}

void
TrnFSM::updateTransaction(const TC_End_Ind& dialogueIndicationPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( _transactionState != INIT_SENT && _transactionState != ACTIVE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[%s value=(%d)] for TC_End_Ind primitive");
  _transactionState = IDLE;
}

void
TrnFSM::updateTransaction(const TC_UAbort_Ind& dialogueIndicationPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( _transactionState != INIT_SENT && _transactionState != ACTIVE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[%s value=(%d)] for TC_UAbort_Ind primitive", stateToString(_transactionState), _transactionState);
  _transactionState = IDLE;
}

void
TrnFSM::updateTransaction(const TC_PAbort_Ind& dialogueIndicationPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( _transactionState != INIT_SENT && _transactionState != ACTIVE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[%s value=(%d)] for TC_PAbort_Ind primitive", stateToString(_transactionState), _transactionState);
  _transactionState = IDLE;
}

void
TrnFSM::updateTransaction(const TC_Notice_Ind& dialogueIndicationPrimitive)
{
  smsc::core::synchronization::MutexGuard synchronize(_transactionStateLock);
  if ( _transactionState == IDLE )
    throw utilx::ProtocolException("TrnFSM::updateTransaction::: invalid state=[IDLE] for TC_Notice_Ind primitive");
}

const char* const
TrnFSM::stateToString(transaction_state_t transactionState) const
{
  if ( transactionState == IDLE )
    return "IDLE";
  if ( transactionState == INIT_SENT )
    return "INIT_SENT";
  if ( transactionState == INIT_RECEIVED )
    return "INIT_RECEIVED";
  if ( transactionState == ACTIVE )
    return "ACTIVE";

  return "INVALID";
}

const proto::TransactionId&
TrnFSM::getTransactionId() const
{
  return _trnId;
}

void
TrnFSM::composeTrnId(const proto::TransactionId& remoteTrnId)
{
  _trnId.setIdRemote(remoteTrnId.remoteId());
}

}}}
