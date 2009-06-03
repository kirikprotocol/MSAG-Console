#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/utilx/Exception.hpp"
#include "eyeline/tcap/Exception.hpp"

#include "eyeline/tcap/provd/TCAPIndicationsProcessor.hpp"
#include "eyeline/tcap/provd/AppCtxSMRegistry.hpp"
#include "eyeline/tcap/provd/TDialogueServiceDataRegistry.hpp"
#include "eyeline/tcap/provd/P_U_AbortPrimitiveUtils.hpp"
#include "eyeline/tcap/provd/TCAPLayer.hpp"
#include "eyeline/tcap/provd/TCAPConfiguration.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

TCAPIndicationsProcessor::TCAPIndicationsProcessor()
  : _logger(smsc::logger::Logger::getInstance("tcap.provd")),
    _ownAddress(TCAPConfiguration::getInstance().getOwnAddress())
{}

bool
TCAPIndicationsProcessor::updateDialogue(TBeginIndComposer & tc_begin_ind, unsigned int src_link_num)
{
  try {
    AppCtxSMRegistry::RegistryEntry regEntry =
      AppCtxSMRegistry::getInstance().getDialogueHandler(*tc_begin_ind.getAppCtx());

    try {
      TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData =
        TDialogueServiceDataRegistry::getInstance().createTDialogueServiceData(regEntry.dialogueHandler,
                                                                               tc_begin_ind.getTransactionId(),
                                                                               regEntry.dialogueTimeout);
      tDlgSvcData->setLinkNum(src_link_num);
      tDlgSvcData->updateDialogueDataByIndication(tc_begin_ind);

      return true;
    } catch (std::exception& ex) {
      formPAbortRequest(tc_begin_ind.getTransactionId(), PAbort::p_resourceLimitation,
                        src_link_num, tc_begin_ind.getDestAddress(), tc_begin_ind.getOrigAddress());

      return false;
    }
  } catch (utilx::RegistryKeyNotFound& ex) {
    formUAbortRequest(*tc_begin_ind.getAppCtx(), tc_begin_ind.getTransactionId(), src_link_num,
                      tc_begin_ind.getDestAddress(), tc_begin_ind.getOrigAddress());
  }
  return false;
}

bool
TCAPIndicationsProcessor::updateDialogue(TContIndComposer & tc_cont_ind, unsigned int src_link_num)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tc_cont_ind.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(tc_cont_ind);

    return true;
  } catch (UnknownDialogueException& ex) {
    formPAbortRequest(tc_cont_ind.getTransactionId(), PAbort::p_unrecognizedTransactionID, src_link_num, tc_cont_ind.getDestAddress(), tc_cont_ind.getOrigAddress());
  } catch (std::exception& ex) {
    formPAbortRequest(tc_cont_ind.getTransactionId(), PAbort::p_resourceLimitation, src_link_num, tc_cont_ind.getDestAddress(), tc_cont_ind.getOrigAddress());
  }

  return false;
}

bool
TCAPIndicationsProcessor::updateDialogue(TEndIndComposer & tc_end_ind, unsigned int src_link_num)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tc_end_ind.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(tc_end_ind);

    return true;
  } catch (UnknownDialogueException& ex) {
  } catch (std::exception& ex) {
  }

  return false;
}

bool
TCAPIndicationsProcessor::updateDialogue(TPAbortIndComposer & tc_pAbort_ind, unsigned int src_link_num)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tc_pAbort_ind.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(tc_pAbort_ind);

    return true;
  } catch (UnknownDialogueException& ex) {
  } catch (std::exception& ex) {
  }

  return false;
}

bool
TCAPIndicationsProcessor::updateDialogue(TUAbortIndComposer & tc_uAbort_ind, unsigned int src_link_num)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tc_uAbort_ind.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(tc_uAbort_ind);

    return true;
  } catch (UnknownDialogueException& ex) {
  } catch (std::exception& ex) {
  }

  return false;
}

bool
TCAPIndicationsProcessor::updateDialogue(TNoticeIndComposer & tc_notice_ind, unsigned int src_link_num)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tc_notice_ind.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(tc_notice_ind);

    return true;
  } catch (UnknownDialogueException& ex) {
  } catch (std::exception& ex) {
  }

  return false;
}

}}}
