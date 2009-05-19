#ifndef __EYELINE_TCAP_PROVD_PUABORTPRIMITIVEUTILS_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_PUABORTPRIMITIVEUTILS_HPP__

# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TDlgHandlerIface.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"
# include "eyeline/tcap/TDialogueHandlingPrimitive.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void formPAbortRequest(const proto::TransactionId& trnId,
                       TDialogueHandlingPrimitive::PAbortCause_e pAbortCause);

void formPAbortRequest(const proto::TransactionId& trn_id,
                       TDialogueHandlingPrimitive::PAbortCause_e p_abort_cause,
                       unsigned int src_link_num,
                       const sccp::SCCPAddress& src_addr,
                       const sccp::SCCPAddress& dst_addr);

void formPAbortIndication(const TDialogueId& dlgId,
                          TDialogueHandlingPrimitive::PAbortCause_e pAbortCause,
                          TDlgHandlerIface* tDlgHndlrIface);

void formUAbortRequest(const asn1::EncodedOID& appCtx,
                       const proto::TransactionId& trnId,
                       unsigned int srcLinkNum,
                       const sccp::SCCPAddress& src_addr,
                       const sccp::SCCPAddress& dst_addr );

}}}

#endif
