#ifndef __EYELINE_TCAP_PROVD_PUABORTPRIMITIVEUTILS_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_PUABORTPRIMITIVEUTILS_HPP__

# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TDlgHandlerIface.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"
# include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"
# include "eyeline/tcap/provd/TDialogueServiceData.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void formPAbortRequest(const proto::TransactionId& trn_id,
                       TDialogueHandlingPrimitive::PAbortCause_e p_abort_cause);

void formPAbortRequest(const proto::TransactionId& trn_id,
                       TDialogueHandlingPrimitive::PAbortCause_e p_abort_cause,
                       unsigned int src_link_num,
                       const sccp::SCCPAddress& src_addr,
                       const sccp::SCCPAddress& dst_addr);

void formPAbortIndication(const TDialogueId& dlg_id,
                          TDialogueHandlingPrimitive::PAbortCause_e p_abort_cause,
                          TDlgHandlerIface* t_dlg_hndlr_iface);

void formPAbortIndication(const TDialogueId& dlg_id,
                          TDialogueHandlingPrimitive::PAbortCause_e p_abort_cause,
                          TDialogueServiceData* t_dlg_svc_data);

void formUAbortRequest(const asn1::EncodedOID& app_ctx,
                       const proto::TransactionId& trn_id,
                       unsigned int src_link_num,
                       const sccp::SCCPAddress& src_addr,
                       const sccp::SCCPAddress& dst_addr);

}}}

#endif
