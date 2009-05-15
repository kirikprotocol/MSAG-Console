#include "util/Exception.hpp"
#include "TDialogueServiceData.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class DialogueTimeoutHandler : public TimeoutHandler {
public:
  explicit DialogueTimeoutHandler(TDialogueServiceData* tDlgSvcData)
    : _tDlgSvcData(tDlgSvcData) {}

  virtual void handle() {
    _tDlgSvcData->notifyDialogueTimeoutExpiration();
  }
private:
  TDialogueServiceData* _tDlgSvcData;
};

class InvocationTimeoutHandler : public TimeoutHandler {
public:
  InvocationTimeoutHandler(TDialogueServiceData* tDlgSvcData, uint8_t invokeId)
    : _tDlgSvcData(tDlgSvcData), _invokeId(invokeId) {}

  virtual void handle() {
    _tDlgSvcData->notifyInvocationTimeoutExpiration(_invokeId);
  }
private:
  TDialogueServiceData* _tDlgSvcData;
  uint8_t _invokeId;
};

TDialogueServiceData::TDialogueServiceData(const TDialogueId& dialogueId,
                                           uint32_t localTrnId,
                                           TDlgHandlerIface* dlgHndlrIface,
                                           unsigned int dialogueTimeout)
  : _dialogueId(dialogueId), _trnFSM(localTrnId),
    _tDlgHndlrIface(dlgHndlrIface), _needReleaseDlgHndlr(false),
    _isDialogueWasAcked(false), _applicationContext(NULL),
    _linkNum(0), _isSetLinkNum(false), _dialogueTimeout(dialogueTimeout),
    _dialogueTimeoutId(0), _isSetDialogueTimeoutId(false)
{
  memset(reinterpret_cast<uint8_t*>(_operationTimers), 0, sizeof(_operationTimers));
}

TDialogueServiceData::TDialogueServiceData(const TDialogueId& dialogueId,
                                           uint32_t localTrnId,
                                           uint32_t remoteTrnId,
                                           TDlgHandlerIface* dlgHndlrIface,
                                           unsigned int dialogueTimeout)
  : _dialogueId(dialogueId), _trnFSM(localTrnId, remoteTrnId),
    _tDlgHndlrIface(dlgHndlrIface), _needReleaseDlgHndlr(true),
    _isDialogueWasAcked(false), _applicationContext(NULL),
    _linkNum(0), _isSetLinkNum(false), _dialogueTimeout(dialogueTimeout),
    _dialogueTimeoutId(0), _isSetDialogueTimeoutId(false)
{}

TDialogueServiceData::~TDialogueServiceData()
{
  if ( _needReleaseDlgHndlr )
    delete _tDlgHndlrIface;
}

void
TDialogueServiceData::updateDialogueDataByRequest(TC_Begin_Req* beginReqPrimitive)
{
  handleDialogueRequestPrimitive(beginReqPrimitive);
}

void
TDialogueServiceData::updateDialogueDataByRequest(TC_Cont_Req* contReqPrimitive)
{
  handleDialogueRequestPrimitive(contReqPrimitive);
}

void
TDialogueServiceData::updateDialogueDataByRequest(TC_End_Req* endReqPrimitive)
{
  try {
    handleDialogueRequestPrimitive(endReqPrimitive);
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(endReqPrimitive->getDialogueId());
  } catch (...) { 
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(endReqPrimitive->getDialogueId());
    throw;
  }
}

void
TDialogueServiceData::updateDialogueDataByRequest(TC_UAbort_Req* uAbortReqPrimitive)
{
  try {
    handleDialogueRequestPrimitive(uAbortReqPrimitive);
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(uAbortReqPrimitive->getDialogueId());
  } catch (...) { 
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(uAbortReqPrimitive->getDialogueId());
    throw;
  }
}

void
TDialogueServiceData::updateDialogueDataByRequest(TC_PAbort_Req* pAbortReqPrimitive)
{
  try {
    handleDialogueRequestPrimitive(pAbortReqPrimitive);
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(pAbortReqPrimitive->getDialogueId());
  } catch (...) { 
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(pAbortReqPrimitive->getDialogueId());
    throw;
  }
}

void
TDialogueServiceData::updateDialogueDataByIndication(TC_Begin_Ind* tcBeginIndPrimitive)
{
  _trnFSM.updateTransaction(*tcBeginIndPrimitive);

  tcBeginIndPrimitive->setDialogueId(getDialogueId());

  if ( tcBeginIndPrimitive->getAppCtx() ) {
    smsc::core::synchronization::MutexGuard synchronize(_lock_forAppCtxUpdate);
    _applicationContext = *tcBeginIndPrimitive->getAppCtx();
  }

  setDialogueTimeoutId(TimeoutMonitor::getInstance().schedule(getDialogueTimeout(), new DialogueTimeoutHandler(this)));

  notifyTCUser(tcBeginIndPrimitive);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TC_Cont_Ind* tcContIndPrimitive)
{
  handleInvocationResults(tcContIndPrimitive->CompList());
  handleDialogueNotTerminationIndPrimitive(tcContIndPrimitive);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TC_End_Ind* tcEndIndPrimitive)
{
  handleInvocationResults(tcEndIndPrimitive->CompList());
  handleDialogueTerminationIndPrimitive(tcEndIndPrimitive);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TC_PAbort_Ind* tcPAbortIndPrimitive)
{
  handleDialogueTerminationIndPrimitive(tcPAbortIndPrimitive);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TC_UAbort_Ind* tcUAbortIndPrimitive)
{
  handleDialogueTerminationIndPrimitive(tcUAbortIndPrimitive);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TC_Notice_Ind* tcNoticeIndPrimitive)
{
  handleDialogueNotTerminationIndPrimitive(tcNoticeIndPrimitive);
}

const proto::TransactionId&
TDialogueServiceData::getTransactionId() const
{
  return _trnFSM.getTransactionId();
}

const TDialogueId&
TDialogueServiceData::getDialogueId() const
{
  return _dialogueId;
}

unsigned int
TDialogueServiceData::getLinkNum() const
{
  if ( _isSetLinkNum )
    return _linkNum;
  else
    throw smsc::util::Exception("TDialogueServiceData::getLinkNum::: link number is not set");
}

void
TDialogueServiceData::setLinkNum(unsigned int linkNum)
{
  _linkNum = linkNum;
  _isSetLinkNum = true;
}

unsigned int
TDialogueServiceData::getDialogueTimeout() const
{
  return _dialogueTimeout;
}

void
TDialogueServiceData::setDialogueTimeoutId(TimeoutMonitor::timeout_id_t timeoutId)
{
  _dialogueTimeoutId = timeoutId;
  _isSetDialogueTimeoutId = true;
}

TimeoutMonitor::timeout_id_t
TDialogueServiceData::getDialogueTimeoutId()
{
  if ( _isSetDialogueTimeoutId )
    return _dialogueTimeoutId;
  else
    throw smsc::util::Exception("TDialogueServiceData::getDialogueTimeoutId::: timeout id is not set");
}

void
TDialogueServiceData::activateDialogueTimer()
{
  setDialogueTimeoutId(TimeoutMonitor::getInstance().schedule(getDialogueTimeout(), new DialogueTimeoutHandler(this)));
}

void
TDialogueServiceData::cancelDialogueTimers()
{
  TimeoutMonitor::getInstance().cancel(getDialogueTimeoutId());
  for (uint8_t invokeId=0; invokeId<=255; ++invokeId)
    cancelInvocationTimer(invokeId);
}

void
TDialogueServiceData::notifyDialogueTimeoutExpiration()
{
  TC_PAbort_Ind pAbortInd;

  pAbortInd.setDialogueId(getDialogueId());
  pAbortInd.setPAbortCause(TDialogueHandlingPrimitive::p_dialogueTimeout);

  try {
    smsc::core::synchronization::MutexGuard synchronize(_lock_forCallToTDlgHndlrIface);
    _tDlgHndlrIface->updateDialogue(pAbortInd);
  } catch (...) {}
}

void
TDialogueServiceData::notifyInvocationTimeoutExpiration(uint8_t invokeId)
{
  TC_L_Cancel_Ind tcLCancel;
  tcLCancel.setDialogueId(getDialogueId());
  tcLCancel.setInvokeId(invokeId);

  {
    smsc::core::synchronization::MutexGuard synchronize(_lock_forOperationTimers);
    _operationTimers[invokeId] = 0;
  }

  try {
    smsc::core::synchronization::MutexGuard synchronize(_lock_forCallToTDlgHndlrIface);
    _tDlgHndlrIface->updateDialogue(tcLCancel);
  } catch (...) {}
}

void
TDialogueServiceData::activateInvocationTimer(uint16_t invokeTimeout, uint8_t invokeId)
{
  smsc::core::synchronization::MutexGuard synchronize(_lock_forOperationTimers);
  _operationTimers[invokeId] = TimeoutMonitor::getInstance().schedule(invokeTimeout, new InvocationTimeoutHandler(this, invokeId));
}

void
TDialogueServiceData::cancelInvocationTimer(uint8_t invokeId)
{
  smsc::core::synchronization::MutexGuard synchronize(_lock_forOperationTimers);
  if ( _operationTimers[invokeId] ) {
    TimeoutMonitor::getInstance().cancel(_operationTimers[invokeId]);
    _operationTimers[invokeId] = 0;
  }
}

void
TDialogueServiceData::handleInvocationResults(ROSComponentsList& componentsList)
{
  for (ros::ROSComponentsList::const_iterator it = componentsList.begin(); it != componentsList.end(); ++it) {
    ros::ROSComponentPrimitive* rosCompPrimitive = it->get();
    if ( rosCompPrimitive ) {
      ros::ROSComponentPrimitive::Kind_e rosKind = rosCompPrimitive->rosKind();
      if ( rosKind == ros::ROSComponentPrimitive::rosResult ||
           rosKind == ros::ROSComponentPrimitive::rosError ||
           rosKind == ros::ROSComponentPrimitive::rosReject )
        cancelInvocationTimer(rosCompPrimitive->getInvokeId());
    }
  }
}

}}}
