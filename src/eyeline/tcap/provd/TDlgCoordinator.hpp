/* ************************************************************************** *
 * TCAP Dialogue Coordinator: provides interaction between dialogueFSM, local 
 * TC User and remote TC User. 
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLG_COORDINATOR_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLG_COORDINATOR_HPP

#include "logger/Logger.h"

#include "eyeline/ss7na/libsccp/SccpApi.hpp"

#include "eyeline/tcap/provd/TDlgFSMRegistry.hpp"
#include "eyeline/tcap/provd/TDlgIndGenerator.hpp"


namespace eyeline {
namespace tcap {
namespace provd {

class TDlgCoordinator : public TDlgCoordinatorIface, public TDlgReqHandlerIface {
protected:
  TDlgFSMRegistry *         _dlgReg;
  TCLocalIndGenerator *     _indGenerator;
  ss7na::libsccp::SccpApi * _sccpApi;
  smsc::logger::Logger *    _logger;

  // ----------------------------------
  //TCRCode_e updateDialogue(const TR_PAbort_Req & tr_req)  /*throw()*/;

  //Sends UDT to remote peer and accordingly updates associated TCAP dialogue state
  template <class TDLG_REQ_COMPOSER>
    TCRCode_e updateDlgByRequest(TDLG_REQ_COMPOSER & treq_comp, bool send_udt) /*throw()*/;

public:
  TDlgCoordinator(TDlgFSMRegistry & dlg_reg, TCLocalIndGenerator & ind_gen, Logger * use_log = NULL)
    : _dlgReg(&dlg_reg), _indGenerator(&ind_gen), _sccpApi(0)
    , _logger(use_log ? use_log : smsc::logger::Logger::getInstance("tcap.provd"))
  { }
  ~TDlgCoordinator()
  { }

  //Binds Signalling Connection Service Provider
  void initSCSP(SccpApi * sccp_api) { _sccpApi = sccp_api; }

  // --------------------------------------------------------
  // -- TDlgCoordinatorIface interface methods implementation
  // --------------------------------------------------------
  //Releases local transaction with no notification sent to remote peer.
  virtual void rlseDialogue(TDialogueId dlg_id) /*throw()*/;
  
  //Starts an indication task that notifies local TSM & TC User about message sending failure
  virtual TCRCode_e notifyLclTCUser(const TransactionId & tr_id,
                                    const SCSPUnitdataReq & req_udt) /*throw()*/;
  //Starts an indication task that notifies local TSM & TC User about Provider abort
  virtual TCRCode_e notifyLclTCUser(const TransactionId & tr_id,
                                    PAbort::Cause_e abrt_cause,
                                    unsigned int scsp_link_num) /*throw()*/;
  //Starts an indication task that notifies local TSM & TC User about Invocation timeout
  virtual TCRCode_e notifyLclTCUser(const TransactionId & tr_id,
                                    uint8_t inv_id) /*throw()*/;
  //
  virtual TCRCode_e notifyRmtTCUser(const TransactionId & tr_id,
                                    SCSPUnitdataReq req_udt,
                                    unsigned int scsp_link_num) /*throw()*/;
  //
  virtual TCRCode_e notifyRmtTCUser(const TransactionId & tr_id,
                                    PAbort::Cause_e abrt_cause,
                                    unsigned int scsp_link_num,
                                    const SCCPAddress & src_addr, const SCCPAddress & dst_addr) /*throw()*/;
  //
  virtual TCRCode_e notifyRmtTCUser(const TransactionId & tr_id, 
                                    TDialogueAssociate::DiagnosticUser_e abrt_cause,
                                    unsigned int scsp_link_num,
                                    const SCCPAddress & src_addr, const SCCPAddress & dst_addr) /*throw()*/;

  // --------------------------------------------------------
  // -- TDlgReqHandlerIface interface methods implementation
  // --------------------------------------------------------
  // -- Dialogue(Component contained) handling request primitives handling
  virtual TCRCode_e updateDialogue(const TR_Begin_Req & tr_req) /*throw()*/;
  virtual TCRCode_e updateDialogue(const TR_Cont_Req & tr_req) /*throw()*/;
  virtual TCRCode_e updateDialogue(const TR_End_Req & tr_req) /*throw()*/;
  virtual TCRCode_e updateDialogue(const TR_UAbort_Req & tr_req) /*throw()*/;

  // -- Component only handling request primitives handling
  virtual TCRCode_e updateDialogue(const TC_U_Cancel_Req & tc_req) /*throw()*/;
  virtual TCRCode_e updateDialogue(const TC_Timer_Reset_Req & tc_req) /*throw()*/;
};


#include "eyeline/tcap/provd/TDlgCoordinator_impl.hpp"

}}}

#endif /* __EYELINE_TCAP_PROVD_TDLG_COORDINATOR_HPP */

