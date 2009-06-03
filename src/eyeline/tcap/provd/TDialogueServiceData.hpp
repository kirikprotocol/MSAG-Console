#ifndef __EYELINE_TCAP_PROVD_TDIALOGUESERVICEDATA_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TDIALOGUESERVICEDATA_HPP__

# include "logger/Logger.h"

# include "core/synchronization/Mutex.hpp"
# include "core/synchronization/MutexGuard.hpp"

# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"

# include "eyeline/tcap/TDlgHandlerIface.hpp"

# include "eyeline/tcap/provd/TDlgIndComposers.hpp"
# include "eyeline/tcap/provd/TDlgReqComposers.hpp"
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
                       unsigned int dialogueTimeout,
                       const sccp::SCCPAddress& ownAddress);

  TDialogueServiceData(const TDialogueId& dialogueId,
                       uint32_t localTrnId,
                       uint32_t remoteTrnId,
                       TDlgHandlerIface* dlgHndlrIface,
                       unsigned int dialogueTimeout);

  ~TDialogueServiceData();

  const proto::TransactionId& getTransactionId() const;
  const TDialogueId& getDialogueId() const;

  void updateDialogueDataByRequest(TBeginReqComposer & treq_begin);
  void updateDialogueDataByRequest(TContReqComposer & treq_cont);
  void updateDialogueDataByRequest(TEndReqComposer & treq_end);
  void updateDialogueDataByRequest(TUAbortReqComposer & treq_uAbort);
  void updateDialogueDataByRequest(TPAbortReqComposer & treq_pAbort);

  void updateDialogueDataByIndication(TBeginIndComposer & tind_begin);
  void updateDialogueDataByIndication(TContIndComposer & tind_cont);
  void updateDialogueDataByIndication(TEndIndComposer & tind_end);
  void updateDialogueDataByIndication(TPAbortIndComposer & tind_pAbort);
  void updateDialogueDataByIndication(TUAbortIndComposer & tind_uAbort);
  void updateDialogueDataByIndication(TNoticeIndComposer & tind_notice);

  unsigned int getLinkNum() const;
  void setLinkNum(unsigned int linkNum);

  const sccp::SCCPAddress& getSrcAddr() const;
  const sccp::SCCPAddress& getDstAddr() const;

  unsigned int getDialogueTimeout() const;

  void activateDialogueTimer();
  void cancelDialogueTimers();

  void notifyDialogueTimeoutExpiration();
  void notifyInvocationTimeoutExpiration(uint8_t invokeId);

  void activateInvocationTimer(uint16_t invokeTimeout, uint8_t invokeId);
  void cancelInvocationTimer(uint8_t invokeId);

  template<class T_COMP_IND_PRIMITIVE>
  void notifyTCUserLocally(T_COMP_IND_PRIMITIVE & tc_comp_primitive);
protected:
  template<class T_DIALOGUE_TERMINATION_IND_PRIMITIVE>
  void handleDialogueTerminationIndPrimitive(T_DIALOGUE_TERMINATION_IND_PRIMITIVE & tind_term_primitive);

  template<class T_DIALOGUE_NOT_TERMINATION_IND_PRIMITIVE>
  void handleDialogueNotTerminationIndPrimitive(T_DIALOGUE_NOT_TERMINATION_IND_PRIMITIVE & tind_not_term_primitive);

  void handleInvocationResults(const ROSComponentsList * componentsList);

  TimeoutMonitor::timeout_id_t getDialogueTimeoutId();
  void setDialogueTimeoutId(TimeoutMonitor::timeout_id_t timeoutId);

  template<class T_DLG_REQUEST_COMPOSER>
  void handleDialogueRequestPrimitive(T_DLG_REQUEST_COMPOSER & treq_composer);

  template<class T_DIALOGUE_IND_PRIMITIVE>
  void notifyTCUser(T_DIALOGUE_IND_PRIMITIVE & tc_ind_primitive);

  void setDstAddr(const sccp::SCCPAddress & dst_addr);
  void setSrcAddr(const sccp::SCCPAddress & dst_addr);

  void checkAndProcessFirstDialogueResponse(TDlgRequestComposerAC* tc_req);

  friend class TrnFSM;
private:
  smsc::logger::Logger* _logger;
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

  sccp::SCCPAddress _dstAddr, _srcAddr;
  bool _isSetDstAddr, _isSetSrcAddr;

  smsc::core::synchronization::Mutex _lock_forCallToTDlgHndlrIface;
  mutable smsc::core::synchronization::Mutex _lock_forAppCtxUpdate;

  smsc::core::synchronization::Mutex _lock_forOperationTimers;
  TimeoutMonitor::timeout_id_t _operationTimers[256];

  smsc::core::synchronization::Mutex _lock_firstIncomingDlrRsp;
  bool _isFirstResponseToIncomingDialogue;
};

# include "eyeline/tcap/provd/TDialogueServiceData_impl.hpp"

}}}

#endif
