/* ************************************************************************** *
 * TCAP Dialogue Coordinator: interface definition.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLG_COORDINATOR_IFACE_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLG_COORDINATOR_IFACE_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/tcap/TcapErrors.hpp"

#include "eyeline/tcap/provd/TransactionId.hpp"
#include "eyeline/tcap/provd/SCSPRequests.hpp"

#include "eyeline/ss7na/libsccp/SccpApi.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::tcap::TCError;
using eyeline::tcap::TDialogueId;
using eyeline::tcap::TDlgTimeout;
using eyeline::tcap::TDialogueAssociate;
using eyeline::tcap::PAbort;

using eyeline::sccp::SCCPAddress;
using eyeline::ss7na::libsccp::SccpApi;


class TDlgCoordinatorIface {
protected:
  virtual ~TDlgCoordinatorIface();

public:
  //Returns false if dialogue cann't be created due to resource limitations
  //virtual bool initDialogue(TDialogueId & dlg_id, const TDlgLocalConfig & dlg_cfg) /*throw()*/ = 0;
  //virtual bool initDialogue(TDialogueId & dlg_id, const TDlgRemoteConfig & dlg_cfg) /*throw()*/ = 0;

  //Releases local transaction with no notification sent to remote peer.
  virtual void rlseDialogue(TDialogueId dlg_id) /*throw()*/ = 0;
  //
  //Starts an indication task that notifies local TSM & TC User about message sending failure
  virtual TCRCode_e notifyLclTCUser(const TransactionId & tr_id,
                                    const SCSPUnitdataReq & req_udt) /*throw()*/ = 0;
  //Starts an indication task that notifies local TSM & TC User about Provider abort
  virtual TCRCode_e notifyLclTCUser(const TransactionId & tr_id,
                                    PAbort::Cause_e abrt_cause,
                                    unsigned int scsp_link_num) /*throw()*/ = 0;
  //Starts an indication task that notifies local TSM & TC User about Invocation timeout
  virtual TCRCode_e notifyLclTCUser(const TransactionId & tr_id,
                                    uint8_t inv_id) /*throw()*/ = 0;
  //
  virtual TCRCode_e notifyRmtTCUser(const TransactionId & tr_id,
                                    SCSPUnitdataReq req_udt,
                                    unsigned int scsp_link_num) /*throw()*/ = 0;
  //
  virtual TCRCode_e notifyRmtTCUser(const TransactionId & tr_id,
                                    PAbort::Cause_e abrt_cause,
                                    unsigned int scsp_link_num,
                                    const SCCPAddress & src_addr, const SCCPAddress & dst_addr) /*throw()*/ = 0;
  //
  virtual TCRCode_e notifyRmtTCUser(const TransactionId & tr_id,
                                    TDialogueAssociate::DiagnosticUser_e abrt_cause,
                                    unsigned int scsp_link_num,
                                    const SCCPAddress & src_addr, const SCCPAddress & dst_addr) /*throw()*/ = 0;

};

}}}

#endif /* __EYELINE_TCAP_PROVD_TDLG_COORDINATOR_IFACE_HPP */

