#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/utilx/Exception.hpp"
#include "eyeline/tcap/Exception.hpp"

#include "eyeline/tcap/provd/TCAPIndicationsProcessor.hpp"
#include "eyeline/tcap/provd/AppCtxSMRegistry.hpp"
#include "eyeline/tcap/provd/TDlgReqSender.hpp"
#include "eyeline/tcap/provd/TDialogueServiceDataRegistry.hpp"
#include "eyeline/tcap/provd/P_U_AbortPrimitiveUtils.hpp"
#include "eyeline/tcap/provd/TCAPLayer.hpp"
#include "eyeline/tcap/provd/TCAPConfiguration.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

TCAPIndicationsProcessor::TCAPIndicationsProcessor()
  : _ownAddress(TCAPConfiguration::getInstance().getOwnAddress())
{}

bool
TCAPIndicationsProcessor::updateDialogue(TC_Begin_Ind& tc_begin_ind, unsigned int src_link_num)
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
      tDlgSvcData->updateDialogueDataByIndication(&tc_begin_ind);

      return true;
    } catch (std::exception& ex) {
      formPAbortRequest(tc_begin_ind.getTransactionId(), TDialogueHandlingPrimitive::p_resourceLimitation,
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
TCAPIndicationsProcessor::updateDialogue(TC_Cont_Ind& tcContInd, unsigned int srcLinkNum)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tcContInd.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(&tcContInd);

    return true;
  } catch (UnknownDialogueException& ex) {
    formPAbortRequest(tcContInd.getTransactionId(), TDialogueHandlingPrimitive::p_unrecognizedTransactionID, srcLinkNum, tcContInd.getDestAddress(), tcContInd.getOrigAddress());
  } catch (std::exception& ex) {
    formPAbortRequest(tcContInd.getTransactionId(), TDialogueHandlingPrimitive::p_resourceLimitation, srcLinkNum, tcContInd.getDestAddress(), tcContInd.getOrigAddress());
  }

  return false;
}

bool
TCAPIndicationsProcessor::updateDialogue(TC_End_Ind& tcEndInd, unsigned int srcLinkNum)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tcEndInd.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(&tcEndInd);

    return true;
  } catch (UnknownDialogueException& ex) {
  } catch (std::exception& ex) {
  }

  return false;
}

bool
TCAPIndicationsProcessor::updateDialogue(TC_PAbort_Ind& tcPAbortInd, unsigned int srcLinkNum)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tcPAbortInd.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(&tcPAbortInd);

    return true;
  } catch (UnknownDialogueException& ex) {
  } catch (std::exception& ex) {
  }

  return false;
}

bool
TCAPIndicationsProcessor::updateDialogue(TC_UAbort_Ind& tcUAbortInd, unsigned int srcLinkNum)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tcUAbortInd.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(&tcUAbortInd);

    return true;
  } catch (UnknownDialogueException& ex) {
  } catch (std::exception& ex) {
  }

  return false;
}

bool
TCAPIndicationsProcessor::updateDialogue(TC_Notice_Ind& tcNoticeInd, unsigned int srcLinkNum)
{
  try {
    TDialogueServiceDataRegistry::registry_element_ref_t tDlgSvcData = TDialogueServiceDataRegistry::getInstance().getTDialogueServiceData(tcNoticeInd.getTransactionId());

    tDlgSvcData->updateDialogueDataByIndication(&tcNoticeInd);

    return true;
  } catch (UnknownDialogueException& ex) {
  } catch (std::exception& ex) {
  }

  return false;
}

}}}
