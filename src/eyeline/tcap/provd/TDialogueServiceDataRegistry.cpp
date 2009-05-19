#include "TDialogueServiceDataRegistry.hpp"
#include "core/synchronization/MutexGuard.hpp"
#include "eyeline/tcap/Exception.hpp"
#include "TCAPConfiguration.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

TDialogueServiceDataRegistry::TDialogueServiceDataRegistry()
  : _localTrnId(0), _dialogueId(0)
{}

TDialogueServiceDataRegistry::registry_element_ref_t
TDialogueServiceDataRegistry::createTDialogueServiceData(TDlgHandlerIface* tIndicationHandlers,
                                                         unsigned int dialogueTimeout)
{
  uint32_t localTrnId;
  TDialogueId dlgId;

  allocateDialogueIdAndLocalTransactionId(&localTrnId, &dlgId);

  registry_element_ref_t tDlgSvcData(new TDialogueServiceData(dlgId, localTrnId,
                                                              tIndicationHandlers, dialogueTimeout,
                                                              TCAPConfiguration::getInstance().getOwnAddress()));
  setSearchKeys(proto::TransactionId(proto::TransactionId::orgLocal, localTrnId),
                dlgId, registry_element_ref_t(tDlgSvcData));

  return tDlgSvcData;
}

TDialogueServiceDataRegistry::registry_element_ref_t
TDialogueServiceDataRegistry::createTDialogueServiceData(TDlgHandlerIface* tIndicationHandlers,
                                                         const proto::TransactionId& remoteTrnId,
                                                         unsigned int dialogueTimeout)
{
  uint32_t localTrnId;
  TDialogueId dlgId;

  allocateDialogueIdAndLocalTransactionId(&localTrnId, &dlgId);

  registry_element_ref_t tDlgSvcData(new TDialogueServiceData(dlgId, localTrnId, remoteTrnId.remoteId(),
                                                              tIndicationHandlers, dialogueTimeout));
  setSearchKeys(tDlgSvcData->getTransactionId(), dlgId, registry_element_ref_t(tDlgSvcData));

  return tDlgSvcData;
}

TDialogueServiceDataRegistry::registry_element_ref_t
TDialogueServiceDataRegistry::getTDialogueServiceData(const TDialogueId& dialogueId)
{
  smsc::core::synchronization::MutexGuard synchronize(_lockForDataStructures);

  dlgId_2_dlgInfo_t::iterator iter = _dlgId2DlgInfo.find(dialogueId);
  if ( iter == _dlgId2DlgInfo.end() )
    throw UnknownDialogueException("TDialogueServiceDataRegistry::getTDialogueServiceData::: can't find element for dialogue id=[%u]", dialogueId);

  return iter->second;
}

TDialogueServiceDataRegistry::registry_element_ref_t
TDialogueServiceDataRegistry::getTDialogueServiceData(const proto::TransactionId& transactionId)
{
  smsc::core::synchronization::MutexGuard synchronize(_lockForDataStructures);

  trnId_2_dlgInfo_t::iterator iter = _trnIdTo2DlgInfo.find(transactionId);
  if ( iter == _trnIdTo2DlgInfo.end() )
    throw UnknownDialogueException("TDialogueServiceDataRegistry::getTDialogueServiceData::: can't find element for transaction id=[%s]", transactionId.uidString().c_str());

  return iter->second;
}

void
TDialogueServiceDataRegistry::destroyTDialogueServiceData(const TDialogueId& dialogueId)
{
  smsc::core::synchronization::MutexGuard synchronize(_lockForDataStructures);

  dlgId_2_dlgInfo_t::iterator iter_1 = _dlgId2DlgInfo.find(dialogueId);
  if ( iter_1 == _dlgId2DlgInfo.end() )
    return;

  trnId_2_dlgInfo_t::iterator iter_2 = _trnIdTo2DlgInfo.find(iter_1->second->getTransactionId());
  if ( iter_2 != _trnIdTo2DlgInfo.end() )
    _trnIdTo2DlgInfo.erase(iter_2);

  _dlgId2DlgInfo.erase(iter_1);
}

void
TDialogueServiceDataRegistry::setSearchKeys(const proto::TransactionId& trnId,
                                            const TDialogueId& dialogueId,
                                            registry_element_ref_t& tDlgSvcData)
{
  smsc::core::synchronization::MutexGuard synchronize(_lockForDataStructures);

  _trnIdTo2DlgInfo.insert(std::make_pair(trnId, tDlgSvcData));
  _dlgId2DlgInfo.insert(std::make_pair(dialogueId, tDlgSvcData));
}

void
TDialogueServiceDataRegistry::allocateDialogueIdAndLocalTransactionId(uint32_t* localTrnId,
                                                                      TDialogueId* dialogueId)
{
  smsc::core::synchronization::MutexGuard synchronize(_lockForIdGeneration);
  *localTrnId = _localTrnId++;
  *dialogueId = TDialogueId(_dialogueId++);
}

}}}
