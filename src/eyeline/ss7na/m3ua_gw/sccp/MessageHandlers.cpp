#include "MessageHandlers.hpp"
#include "scmg/SCMGMessageProcessor.hpp"
#include "eyeline/ss7na/m3ua_gw/types.hpp"
#include "eyeline/ss7na/m3ua_gw/Exception.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/primitives/MTP_Transfer.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"
#include "eyeline/ss7na/m3ua_gw/mtp3/PointsDefinitionRegistry.hpp"
#include "messages/MemoryAllocator.hpp"
#include "SLSGenerator.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

void
MessageHandlers::handle(messages::UDT& message)
{
  try {
    smsc_log_info(_logger, "MessageHandlers::handle::: handle UDT message='%s'", message.toString().c_str());
    if ( checkMessageIsSCMG(message.getProtocolClass(), message.getCallingAddress(),
                            message.getCalledAddress()) ) {
      scmg::SCMGMessageProcessor::getInstance().handle(message.getData());
    } else {
      if (message.getCalledAddress().getRouting() == eyeline::sccp::SCCPAddress::routeSSN)
        handleRouteOnSSN(message);
      else
        handleRouteOnGT(message);
    }
  } catch (common::RouteNotFound& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught RouteNotFound exception");
    sendNegativeResponse(message, ex.getFailureCode());
  } catch (common::SCCPException& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught SCCPException exception='%s'", ex.what());
    sendNegativeResponse(message, ex.getFailureCode());
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught unexpected exception='%s'", ex.what());
    sendNegativeResponse(message, common::SCCP_FAILURE);
  } catch (...) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught unexpected exception='...'");
    sendNegativeResponse(message, common::SCCP_FAILURE);
  }
}

const libsccp::N_UNITDATA_IND_Message*
MessageHandlers::createIndicationMessage(const messages::UDT& message)
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_LIBSCCP_MESSAGE>();
  libsccp::N_UNITDATA_IND_Message* nUnitdataIndMsg =
      new (ptr_desc->allocated_memory) libsccp::N_UNITDATA_IND_Message();

  nUnitdataIndMsg->setSequenceControl(message.getProtocolClass().getProtocolClassValue() == messages::CLASS1_CONNECIONLESS);
  nUnitdataIndMsg->setCalledAddress(message.getCalledAddress());
  nUnitdataIndMsg->setCallingAddress(message.getCallingAddress());
  const utilx::variable_data_t& msgData = message.getData();
  nUnitdataIndMsg->setUserData(msgData.data, msgData.dataLen);
  return nUnitdataIndMsg;
}

void
MessageHandlers::handle(messages::UDTS& message)
{
  try {
    smsc_log_info(_logger, "MessageHandlers::handle::: handle UDTS message='%s'", message.toString().c_str());
    if (message.getCalledAddress().getRouting() == eyeline::sccp::SCCPAddress::routeSSN)
      handleRouteOnSSN(message);
    else
      handleRouteOnGT(message);
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught exception '%s' when handling UDTS message, drop UDTS message",
                   ex.what());
  }
}

const libsccp::N_NOTICE_IND_Message*
MessageHandlers::createIndicationMessage(const messages::UDTS& message)
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_LIBSCCP_MESSAGE>();
  libsccp::N_NOTICE_IND_Message* nNoticeIndMsg =
      new (ptr_desc->allocated_memory) libsccp::N_NOTICE_IND_Message();

  nNoticeIndMsg->setCalledAddress(message.getCalledAddress());
  nNoticeIndMsg->setCallingAddress(message.getCallingAddress());
  nNoticeIndMsg->setReasonForReturn(message.getReturnCause());
  const utilx::variable_data_t& msgData = message.getData();
  nNoticeIndMsg->setUserData(msgData.data, msgData.dataLen);
  return nNoticeIndMsg;
}

void
MessageHandlers::handle(messages::XUDT& message)
{
  try {
    smsc_log_info(_logger, "MessageHandlers::handle::: handle XUDT message='%s'", message.toString().c_str());
    if ( checkMessageIsSCMG(message.getProtocolClass(), message.getCallingAddress(),
                            message.getCalledAddress()) ) {
      scmg::SCMGMessageProcessor::getInstance().handle(message.getData());
    } else {
      if (message.getCalledAddress().getRouting() == eyeline::sccp::SCCPAddress::routeSSN)
        handleRouteOnSSN(message);
      else
        handleRouteOnGT(message);
    }
    return;
  } catch (common::RouteNotFound& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught RouteNotFound exception");
    sendNegativeResponse(message, ex.getFailureCode());
  } catch (common::SCCPException& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught SCCPException exception='%s'", ex.what());
    sendNegativeResponse(message, ex.getFailureCode());
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught unexpected exception='%s'", ex.what());
    sendNegativeResponse(message, common::SCCP_FAILURE);
  } catch (...) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught unexpected exception='...'");
    sendNegativeResponse(message, common::SCCP_FAILURE);
  }

  if ( needReassembling(message) )
    ReassemblyProcessor::getInstance().cancelReassembling(message);
}

const libsccp::N_UNITDATA_IND_Message*
MessageHandlers::createIndicationMessage(const messages::XUDT& message)
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_LIBSCCP_MESSAGE>();
  libsccp::N_UNITDATA_IND_Message* nUnitdataIndMsg =
      new (ptr_desc->allocated_memory) libsccp::N_UNITDATA_IND_Message();

  nUnitdataIndMsg->setSequenceControl(message.getProtocolClass().getProtocolClassValue() == messages::CLASS1_CONNECIONLESS);
  nUnitdataIndMsg->setCalledAddress(message.getCalledAddress());
  nUnitdataIndMsg->setCallingAddress(message.getCallingAddress());
  const utilx::variable_data_t& msgData = message.getData();
  nUnitdataIndMsg->setUserData(msgData.data, msgData.dataLen);
  return nUnitdataIndMsg;
}

const libsccp::N_UNITDATA_IND_Message*
MessageHandlers::createIndicationMessage(const messages::XUDT& message,
                                         const uint8_t* assembled_msg_data,
                                         uint16_t assembled_msg_data_sz)
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_LIBSCCP_MESSAGE>();
  libsccp::N_UNITDATA_IND_Message* nUnitdataIndMsg =
      new (ptr_desc->allocated_memory) libsccp::N_UNITDATA_IND_Message();

  nUnitdataIndMsg->setSequenceControl(message.getProtocolClass().getProtocolClassValue() == messages::CLASS1_CONNECIONLESS);
  nUnitdataIndMsg->setCalledAddress(message.getCalledAddress());
  nUnitdataIndMsg->setCallingAddress(message.getCallingAddress());
  nUnitdataIndMsg->setUserData(assembled_msg_data, assembled_msg_data_sz);
  return nUnitdataIndMsg;
}

void
MessageHandlers::handle(messages::XUDTS& message)
{
  try {
    smsc_log_info(_logger, "MessageHandlers::handle::: handle XUDTS message='%s'", message.toString().c_str());
    if (message.getCalledAddress().getRouting() == eyeline::sccp::SCCPAddress::routeSSN)
      handleRouteOnSSN(message);
    else
      handleRouteOnGT(message);

    return;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught exception '%s' when handling XUDTS message, drop XUDTS message",
                   ex.what());
  }

  if ( needReassembling(message) )
    ReassemblyProcessor::getInstance().cancelReassembling(message);
}

const libsccp::N_NOTICE_IND_Message*
MessageHandlers::createIndicationMessage(const messages::XUDTS& message)
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_LIBSCCP_MESSAGE>();
  libsccp::N_NOTICE_IND_Message* nNoticeIndMsg =
      new (ptr_desc->allocated_memory) libsccp::N_NOTICE_IND_Message();

  nNoticeIndMsg->setCalledAddress(message.getCalledAddress());
  nNoticeIndMsg->setCallingAddress(message.getCallingAddress());
  nNoticeIndMsg->setReasonForReturn(message.getReturnCause());
  const utilx::variable_data_t& msgData = message.getData();
  nNoticeIndMsg->setUserData(msgData.data, msgData.dataLen);
  if (message.isSetImportance() )
    nNoticeIndMsg->setImportance(message.getImportance());

  return nNoticeIndMsg;
}

const libsccp::N_NOTICE_IND_Message*
MessageHandlers::createIndicationMessage(const messages::XUDTS& message,
                                         const uint8_t* assembled_msg_data,
                                         uint16_t assembled_msg_data_sz)
{
  utilx::alloc_mem_desc_t *ptr_desc = utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_LIBSCCP_MESSAGE>();
  libsccp::N_NOTICE_IND_Message* nNoticeIndMsg =
      new (ptr_desc->allocated_memory) libsccp::N_NOTICE_IND_Message();

  nNoticeIndMsg->setCalledAddress(message.getCalledAddress());
  nNoticeIndMsg->setCallingAddress(message.getCallingAddress());
  nNoticeIndMsg->setReasonForReturn(message.getReturnCause());
  nNoticeIndMsg->setUserData(assembled_msg_data, assembled_msg_data_sz);
  if (message.isSetImportance() )
    nNoticeIndMsg->setImportance(message.getImportance());

  return nNoticeIndMsg;
}

bool
MessageHandlers::checkMessageIsSCMG(const messages::ProtocolClass& protocol_class,
                                    const eyeline::sccp::SCCPAddress& calling_address,
                                    const eyeline::sccp::SCCPAddress& called_address)
{
  if ( protocol_class.getProtocolClassValue() != messages::CLASS0_CONNECIONLESS ||
       protocol_class.getRetOnErrorIndication() != messages::NO_SPECIAL_OPTION )
    return false;

  return isSCMGAddrPredicateTrue(calling_address) &&
         isSCMGAddrPredicateTrue(called_address);
}

bool
MessageHandlers::isSCMGAddrPredicateTrue(const eyeline::sccp::SCCPAddress& address)
{
  if ( address.getRouting() == eyeline::sccp::SCCPAddress::routeSSN &&
      address.getIndicator().hasSSN && address.getSSN() == 1 )
    return true;
  else
    return false;
}

void
MessageHandlers::sendNegativeResponse(const messages::UDT& message,
                                      common::return_cause_value_t cause)
{
  if ( message.getProtocolClass().getRetOnErrorIndication() == messages::RETURN_MSG_ON_ERROR ) {
    utilx::alloc_mem_desc_t* mem_header =
        utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_RESPONSE_SCCP_MESSAGE>();
    messages::UDTS* negativeResponse =
        new (mem_header->allocated_memory) messages::UDTS(message, cause);

    smsc_log_debug(_logger, "MessageHandlers::sendNegativeResponse::: prepare negative response='%s'",
                   negativeResponse->toString().c_str());
    throw SCCPFailureException(*negativeResponse, cause);
  }
}

void
MessageHandlers::sendNegativeResponse(const messages::XUDT& message,
                                      common::return_cause_value_t cause)
{
  if ( message.getProtocolClass().getRetOnErrorIndication() == messages::RETURN_MSG_ON_ERROR &&
      isNotSegmentedOrIsFirstSegment(message) ) {
    utilx::alloc_mem_desc_t* mem_header =
        utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_RESPONSE_SCCP_MESSAGE>();
    messages::XUDTS* negativeResponse =
        new (mem_header->allocated_memory) messages::XUDTS(message, cause);

    smsc_log_debug(_logger, "MessageHandlers::sendNegativeResponse::: prepare negative response='%s'",
                   negativeResponse->toString().c_str());
    throw SCCPFailureException(*negativeResponse, cause);
  }
}

void
MessageHandlers::forwardMessageToMTP3(const std::string& route_id,
                                      const messages::SCCPMessage& message)
{
  smsc_log_debug(_logger, "MessageHandlers::forwardMessageToMTP3::: send message='%s' to signalling network, route_id='%s'",
                 message.toString().c_str(), route_id.c_str());
  MTP3SapSelectPolicyRefPtr mtp3SapPolicy =
      PolicyRegistry<MTP3SapSelectPolicy>::getInstance().lookup(route_id);

  common::TP tp;
  message.serialize(&tp);

  uint8_t sls = 0, si = SCCP, msg_prio = 0;
  MTP3SapInfo mtp3SapInfo(0, 0);

  if ( _isSetSequenceControl ) {
    sls = _sequenceControl % 15;
    mtp3SapInfo = mtp3SapPolicy->getValue(_sequenceControl);
  } else {
    sls = SLSGenerator::getInstance().getNextSsl();
    mtp3SapInfo = mtp3SapPolicy->getValue();
  }

  MTP3SapInfo firstMtpSapInfo = mtp3SapInfo;

  do {
    try {
      smsc_log_debug(_logger, "MessageHandlers::forwardMessageToMTP3::: form MTP_Transfer request primitive for lpc/dpc=%u/%u",
                     mtp3SapInfo.getLPC(), mtp3SapInfo.getDPC());
      mtp3::PointInfo localPointInfo =
          mtp3::PointsDefinitionRegistry::getInstance().lookup(mtp3SapInfo.getLPC());

      mtp3::primitives::MTP_Transfer_Req mtpPrimitive(mtp3SapInfo.getLPC(), mtp3SapInfo.getDPC(),
                                                      sls, si, localPointInfo.ni, msg_prio,
                                                      utilx::variable_data_t(tp.packetBody, static_cast<uint16_t>(tp.packetLen)));
      _mtp3SapInstance.handle(mtpPrimitive);
      return;
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "MessageHandlers::forwardMessageToMTP3::: caught exception '%s' while sending MTP_Transfer_Req primitive to MTP3 layer",
                     ex.what());
      mtp3SapInfo = mtp3SapPolicy->getValue();
    }
  } while(mtp3SapInfo != firstMtpSapInfo);

  throw common::TranslationFailure(common::MTP_FAILURE, "MessageHandlers::forwardMessageToMTP3::: no one MTP3Sap is accessible");
}

}}}}
