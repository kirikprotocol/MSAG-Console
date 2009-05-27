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
                                           unsigned int dialogueTimeout,
                                           const sccp::SCCPAddress& ownAddress)
  : _dialogueId(dialogueId), _trnFSM(this, localTrnId),
    _tDlgHndlrIface(dlgHndlrIface), _needReleaseDlgHndlr(false),
    _isDialogueWasAcked(false), _applicationContext(NULL),
    _linkNum(0), _isSetLinkNum(false), _dialogueTimeout(dialogueTimeout),
    _dialogueTimeoutId(0), _isSetDialogueTimeoutId(false), _srcAddr(ownAddress),
    _isSetDstAddr(false), _isSetSrcAddr(false)
{
  memset(reinterpret_cast<uint8_t*>(_operationTimers), 0, sizeof(_operationTimers));
}

TDialogueServiceData::TDialogueServiceData(const TDialogueId& dialogueId,
                                           uint32_t localTrnId,
                                           uint32_t remoteTrnId,
                                           TDlgHandlerIface* dlgHndlrIface,
                                           unsigned int dialogueTimeout)
  : _dialogueId(dialogueId), _trnFSM(this, localTrnId, remoteTrnId),
    _tDlgHndlrIface(dlgHndlrIface), _needReleaseDlgHndlr(true),
    _isDialogueWasAcked(false), _applicationContext(NULL),
    _linkNum(0), _isSetLinkNum(false), _dialogueTimeout(dialogueTimeout),
    _dialogueTimeoutId(0), _isSetDialogueTimeoutId(false), _isSetDstAddr(false), _isSetSrcAddr(false)
{}

TDialogueServiceData::~TDialogueServiceData()
{
  if ( _needReleaseDlgHndlr )
    delete _tDlgHndlrIface;
}

void
TDialogueServiceData::updateDialogueDataByRequest(TBeginReqComposer & treq_begin)
{
  handleDialogueRequestPrimitive(treq_begin);
  setDstAddr(treq_begin.TReq().getDestAddress());
}

void
TDialogueServiceData::updateDialogueDataByRequest(TContReqComposer & treq_cont)
{
  handleDialogueRequestPrimitive(treq_cont);
  setSrcAddr(treq_cont.TReq().getOrigAddress());
}

void
TDialogueServiceData::updateDialogueDataByRequest(TEndReqComposer & treq_end)
{
  try {
    handleDialogueRequestPrimitive(treq_end);
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(treq_end.TReq().getDialogueId());
  } catch (...) { 
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(treq_end.TReq().getDialogueId());
    throw;
  }
}

void
TDialogueServiceData::updateDialogueDataByRequest(TUAbortReqComposer & treq_uAbort)
{
  try {
    handleDialogueRequestPrimitive(treq_uAbort);
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(treq_uAbort.TReq().getDialogueId());
  } catch (...) { 
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(treq_uAbort.TReq().getDialogueId());
    throw;
  }
}

void
TDialogueServiceData::updateDialogueDataByRequest(TPAbortReqComposer & treq_pAbort)
{
  try {
    handleDialogueRequestPrimitive(treq_pAbort);
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(treq_pAbort.TReq().getDialogueId());
  } catch (...) { 
    TDialogueServiceDataRegistry::getInstance().destroyTDialogueServiceData(treq_pAbort.TReq().getDialogueId());
    throw;
  }
}

void
TDialogueServiceData::updateDialogueDataByIndication(TBeginIndComposer & tind_begin)
{
  _trnFSM.updateTransaction(tind_begin);

  tind_begin.setDialogueId(getDialogueId());

  if ( tind_begin.getAppCtx() ) {
    smsc::core::synchronization::MutexGuard synchronize(_lock_forAppCtxUpdate);
    _applicationContext = *tind_begin.getAppCtx();
  }

  setDialogueTimeoutId(TimeoutMonitor::getInstance().schedule(getDialogueTimeout(), new DialogueTimeoutHandler(this)));

  setDstAddr(tind_begin.getOrigAddress());

  notifyTCUser(tind_begin);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TContIndComposer & tind_cont)
{
  handleInvocationResults(tind_cont.CompList());
  handleDialogueNotTerminationIndPrimitive(tind_cont);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TEndIndComposer & tind_end)
{
  handleInvocationResults(tind_end.CompList());
  handleDialogueTerminationIndPrimitive(tind_end);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TPAbortIndComposer & tind_pAbort)
{
  handleDialogueTerminationIndPrimitive(tind_pAbort);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TUAbortIndComposer & tind_uAbort)
{
  handleDialogueTerminationIndPrimitive(tind_uAbort);
}

void
TDialogueServiceData::updateDialogueDataByIndication(TNoticeIndComposer & tc_notice_ind_primitive)
{
  handleDialogueNotTerminationIndPrimitive(tc_notice_ind_primitive);
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
  TPAbortIndComposer pAbortInd;

  pAbortInd.setDialogueId(getDialogueId());
  pAbortInd.setPAbortCause(PAbort::p_dialogueTimeout);

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
TDialogueServiceData::handleInvocationResults(const ROSComponentsList * componentsList)
{
  if (!componentsList || componentsList->empty())
    return;

  for (ros::ROSComponentsList::const_iterator it = componentsList->begin(); it != componentsList->end(); ++it) {
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

const sccp::SCCPAddress&
TDialogueServiceData::getSrcAddr() const
{
  if ( _isSetSrcAddr )
    return _srcAddr;
  else
    throw smsc::util::Exception("TDialogueServiceData::getSrcAddr::: source address is not set");
}

const sccp::SCCPAddress&
TDialogueServiceData::getDstAddr() const
{
  if ( _isSetDstAddr )
    return _dstAddr;
  else
    throw smsc::util::Exception("TDialogueServiceData::getDstAddr::: destination address is not set");
}

void
TDialogueServiceData::setSrcAddr(const sccp::SCCPAddress & src_addr)
{
  _srcAddr = src_addr; _isSetSrcAddr = true;
}

void
TDialogueServiceData::setDstAddr(const sccp::SCCPAddress & dst_addr)
{
  _dstAddr = dst_addr; _isSetDstAddr = true;
}

}}}
