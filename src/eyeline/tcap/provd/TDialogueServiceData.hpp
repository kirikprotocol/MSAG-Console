#ifndef __EYELINE_TCAP_PROVD_TDIALOGUESERVICEDATA_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TDIALOGUESERVICEDATA_HPP__

# include "core/synchronization/Mutex.hpp"
# include "core/synchronization/MutexGuard.hpp"
# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"
# include "eyeline/tcap/TDialogueRequestPrimitives.hpp"
# include "eyeline/tcap/TDialogueIndicationPrimitives.hpp"
# include "eyeline/tcap/TDlgHandlerIface.hpp"

# include "eyeline/tcap/provd/TrnFSM.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"
# include "eyeline/tcap/provd/TimeoutMonitor.hpp"
# include "eyeline/tcap/provd/P_U_AbortPrimitiveUtils.hpp"
# include "eyeline/tcap/provd/TDialogueServiceDataRegistry.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TDialogueServiceData {
public:
  TDialogueServiceData(const TDialogueId& dialogueId,
                       uint32_t trnId,
                       TDlgHandlerIface* dlgHndlrIface,
                       unsigned int dialogueTimeout);

  TDialogueServiceData(const TDialogueId& dialogueId,
                       uint32_t localTrnId,
                       uint32_t remoteTrnId,
                       TDlgHandlerIface* dlgHndlrIface,
                       unsigned int dialogueTimeout);

  ~TDialogueServiceData();

  const proto::TransactionId& getTransactionId() const;
  const TDialogueId& getDialogueId() const;

  void updateDialogueDataByRequest(TC_Begin_Req* beginReqPrimitive);
  void updateDialogueDataByRequest(TC_Cont_Req* contReqPrimitive);
  void updateDialogueDataByRequest(TC_End_Req* endReqPrimitive);
  void updateDialogueDataByRequest(TC_UAbort_Req* uAbortReqPrimitive);
  void updateDialogueDataByRequest(TC_PAbort_Req* pAbortReqPrimitive);

  void updateDialogueDataByIndication(TC_Begin_Ind* tcBeginIndPrimitive);
  void updateDialogueDataByIndication(TC_Cont_Ind* tcContIndPrimitive);
  void updateDialogueDataByIndication(TC_End_Ind* tcEndIndPrimitive);
  void updateDialogueDataByIndication(TC_PAbort_Ind* tcPAbortIndPrimitive);
  void updateDialogueDataByIndication(TC_UAbort_Ind* tcUAbortIndPrimitive);
  void updateDialogueDataByIndication(TC_Notice_Ind* tcNoticeIndPrimitive);

  unsigned int getLinkNum() const;
  void setLinkNum(unsigned int linkNum);

  unsigned int getDialogueTimeout() const;

  void activateDialogueTimer();
  void cancelDialogueTimers();

  void notifyDialogueTimeoutExpiration();
  void notifyInvocationTimeoutExpiration(uint8_t invokeId);

  void activateInvocationTimer(uint16_t invokeTimeout, uint8_t invokeId);
  void cancelInvocationTimer(uint8_t invokeId);

protected:
  template<class T_DIALOGUE_TERMINATION_IND_PRIMITIVE>
  void handleDialogueTerminationIndPrimitive(T_DIALOGUE_TERMINATION_IND_PRIMITIVE* tDlgTermIndPrimitive);

  template<class T_DIALOGUE_NOT_TERMINATION_IND_PRIMITIVE>
  void handleDialogueNotTerminationIndPrimitive(T_DIALOGUE_NOT_TERMINATION_IND_PRIMITIVE* tDlgNotTermIndPrimitive);

  void handleInvocationResults(ROSComponentsList& componentsList);

  TimeoutMonitor::timeout_id_t getDialogueTimeoutId();
  void setDialogueTimeoutId(TimeoutMonitor::timeout_id_t timeoutId);

  template<class T_DIALOGUE_REQUEST_PRIMITIVE>
  void handleDialogueRequestPrimitive(T_DIALOGUE_REQUEST_PRIMITIVE* tDlgReqPrimitive);

  template<class T_DIALOGUE_IND_PRIMITIVE>
  void notifyTCUser(T_DIALOGUE_IND_PRIMITIVE* tcIndPrimitive);

private:
  TDialogueId _dialogueId;

  TrnFSM _trnFSM;
  TDlgHandlerIface* _tDlgHndlrIface;
  bool _needReleaseDlgHndlr, _isDialogueWasAcked;
  asn1::EncodedOID _applicationContext;
  unsigned int _linkNum;
  bool _isSetLinkNum;
  unsigned int _dialogueTimeout;
  TimeoutMonitor::timeout_id_t _dialogueTimeoutId;
  bool _isSetDialogueTimeoutId;

  smsc::core::synchronization::Mutex _lock_forCallToTDlgHndlrIface;
  mutable smsc::core::synchronization::Mutex _lock_forAppCtxUpdate;

  smsc::core::synchronization::Mutex _lock_forOperationTimers;
  TimeoutMonitor::timeout_id_t _operationTimers[256];
};

# include "eyeline/tcap/provd/TDialogueServiceData_impl.hpp"

}}}

#endif
