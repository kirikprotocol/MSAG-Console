#include <algorithm>

#include "MessageHandlers.hpp"
#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"

#include "eyeline/ss7na/m3ua_gw/Exception.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/MessageHandlers.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/messages/ProtocolClass.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/messages/MemoryAllocator.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/sap/ApplicationsRegistry.hpp"
#include "eyeline/ss7na/common/Exception.hpp"
#include "eyeline/ss7na/libsccp/messages/BindConfirmMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace sap {

void
MessageHandlers::handle(const libsccp::BindMessage& message, const common::LinkId& link_id)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle BindMessage=[%s] on link=[%s]", message.toString().c_str(), link_id.getValue().c_str());
  try {
    common::sccp_sap::ActiveAppsRegistry::getInstance().insert(message.getSSNList(),
                                                               message.getSSNList() + message.getNumberOfSSN(),
                                                               link_id, message.getAppId());

    common::LinkId linkSetId;
    if ( makeAuthentication(message.getAppId()) == AUTHENTICATION_OK )
      _cMgr.send(link_id, libsccp::BindConfirmMessage(libsccp::BindConfirmMessage::BIND_OK));
    else
      _cMgr.send(link_id, libsccp::BindConfirmMessage(libsccp::BindConfirmMessage::UNKNOWN_APP_ID_VALUE));
  } catch (utilx::DuplicatedRegistryKeyException& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught DuplicatedRegistryKeyException [%s]", ex.what());
    _cMgr.send(link_id, libsccp::BindConfirmMessage(libsccp::BindConfirmMessage::APP_ALREADY_ACTIVE));
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "MessageHandlers::handle::: caught unexpected exception [%s]", ex.what());
    _cMgr.send(link_id, libsccp::BindConfirmMessage(libsccp::BindConfirmMessage::SYSTEM_MALFUNCTION));
  }

}

void
MessageHandlers::handle(const libsccp::UnbindMessage& message, const common::LinkId& link_id)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle UnbindMessage=[%s]",
                 message.toString().c_str());
  common::sccp_sap::ActiveAppsRegistry::getInstance().remove(link_id);
}

void
MessageHandlers::handle(const libsccp::N_UNITDATA_REQ_Message& message, const common::LinkId& link_id)
{
  smsc_log_info(_logger, "MessageHandlers::handle::: handle N_UNITDATA_REQ_Message=[%s]", message.toString().c_str());

  try {
    if ( decideIfNeedSegmentation(message) ) {
      doSegmentation(message);
    } else {
      sccp::MessageHandlers msgHndlr;
      if ( message.isSetSequenceControl() )
        msgHndlr.setSequenceControl(message.getSequenceControl());

      if ( message.isSetHopCounter() || message.isSetImportance() )
        msgHndlr.handle(formXUDT(message));
      else
        msgHndlr.handle(formUDT(message));
    }
  } catch(SCCPFailureException& ex) {
    try {
      formNegativeResponse(message, ex.getFailureCode(), link_id);
    } catch (...) {}
  } catch (common::SCCPException& ex) {
    try {
      formNegativeResponse(message, ex.getFailureCode(), link_id);
    } catch (...) {}
  } catch (...) {
    try {
      formNegativeResponse(message, common::SCCP_FAILURE, link_id);
    } catch (...) {}
  }
}

bool
MessageHandlers::decideIfNeedSegmentation(const libsccp::N_UNITDATA_REQ_Message& message)
{
  const utilx::variable_data_t& userData = message.getUserData();

  uint16_t totalMsgSize = XUDT_FIXED_FIELDS_LEN + message.getCalledAddress().dataLen +
      MANDATORY_VARIABLE_PARAM_OVERHEAD + message.getCallingAddress().dataLen +
      MANDATORY_VARIABLE_PARAM_OVERHEAD + userData.dataLen +
      MANDATORY_VARIABLE_PARAM_OVERHEAD +
      (message.isSetImportance() ? IMPORTANCE_VALUE_LEN + OPTIONAL_PARAM_FIXED_PREFIX_LEN + END_OF_OPTIONAL_PARAM_SUFFIX_LEN : 0);

  if ( totalMsgSize < MAX_XUDT_LEN )
      return false;

  uint16_t totalMsgSizeAccountWithSegmentParam =
      totalMsgSize + messages::SEGMENTATION_VALUE_SIZE + OPTIONAL_PARAM_FIXED_PREFIX_LEN +
      (message.isSetImportance() ? 0 : END_OF_OPTIONAL_PARAM_SUFFIX_LEN);

  if (  totalMsgSizeAccountWithSegmentParam > MAX_USERDATA_SIZE )
    throw common::SCCPException(common::SEGMENTATION_NOT_SUPPORTED, "MessageHandlers::decideIfNeedSegmentation::: too long user data [%u] - segmentation isn't supported",
                                userData.dataLen);

  return true;
}

void
MessageHandlers::doSegmentation(const libsccp::N_UNITDATA_REQ_Message& message)
{
  smsc_log_debug(_logger, "MessageHandlers::doSegmentation::: make message segmentation");
  const utilx::variable_data_t& userData = message.getUserData();
  const utilx::variable_data_t& callingAddr = message.getCallingAddress();
  const utilx::variable_data_t& calledAddr = message.getCalledAddress();

  uint16_t fixedPartSize = XUDT_FIXED_FIELDS_LEN + calledAddr.dataLen +
      MANDATORY_VARIABLE_PARAM_OVERHEAD + callingAddr.dataLen +
      MANDATORY_VARIABLE_PARAM_OVERHEAD + MANDATORY_VARIABLE_PARAM_OVERHEAD /* overhead for 'data' mandatory variable field*/ +
      OPTIONAL_PART_PTR_SIZE +
      (message.isSetImportance() ? IMPORTANCE_VALUE_LEN + OPTIONAL_PARAM_FIXED_PREFIX_LEN : 0) +
      SEGMENTATION_PARAM_LEN + END_OF_OPTIONAL_PARAM_SUFFIX_LEN;

  uint16_t userDataSizeInEachSegment = 251 - calledAddr.dataLen - callingAddr.dataLen;
  uint16_t totalProcessedUserDataOctets=0;
  uint8_t remainingSegments = userData.dataLen / userDataSizeInEachSegment +
                              (userData.dataLen % userDataSizeInEachSegment ? 1 : 0);
  uint32_t localRef = getUniqueLocaReference();

  smsc_log_debug(_logger, "MessageHandlers::doSegmentation::: calledAddr.dataLen=%u, callingAddr.dataLen=%u, userData.dataLen=%u, fixedPartSize=%u, userDataSizeInEachSegment=%u, remainingSegments=%u",
                 calledAddr.dataLen, callingAddr.dataLen, userData.dataLen, fixedPartSize, userDataSizeInEachSegment, remainingSegments);

  sccp::MessageHandlers msgHndlr;
  msgHndlr.setSequenceControl(localRef);

  while(totalProcessedUserDataOctets < userData.dataLen) {
    uint16_t useDataSizeInNextSegment =
        std::min(static_cast<uint16_t>(userData.dataLen - totalProcessedUserDataOctets),
                 userDataSizeInEachSegment);
    sccp::messages::XUDT& nextXUDT =
        formXUDT(message, totalProcessedUserDataOctets, useDataSizeInNextSegment,
                 --remainingSegments, localRef);
    msgHndlr.handle(nextXUDT);
    totalProcessedUserDataOctets += useDataSizeInNextSegment;
  }
}

sccp::messages::XUDT&
MessageHandlers::formXUDT(const libsccp::N_UNITDATA_REQ_Message& message)
{
  utilx::alloc_mem_desc_t* mem_header =
      utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_SCCP_MESSAGE>();
  sccp::messages::XUDT* xudtMsg =  new (mem_header->allocated_memory) sccp::messages::XUDT();

  messages::protocol_class_t protocolClass = messages::CLASS0_CONNECIONLESS;
  if ( message.isSetSequenceControl() )
    protocolClass = messages::CLASS1_CONNECIONLESS;

  if ( message.getReturnOption() )
    xudtMsg->setProtocolClass(messages::ProtocolClass(protocolClass,
                                                      messages::RETURN_MSG_ON_ERROR));
  else
    xudtMsg->setProtocolClass(messages::ProtocolClass(protocolClass,
                                                      messages::NO_SPECIAL_OPTION));

  if ( message.isSetHopCounter() )
    xudtMsg->setHopCounter(message.getHopCounter());
  else
    xudtMsg->setHopCounter(MAX_HOP_COUNTER_VALUE);

  eyeline::sccp::SCCPAddress calledAddr;
  const utilx::variable_data_t& calledAddrInBinary = message.getCalledAddress();
  calledAddr.unpackOcts(calledAddrInBinary.data, calledAddrInBinary.dataLen);
  xudtMsg->setCalledAddress(calledAddr);

  eyeline::sccp::SCCPAddress callingAddr;
  const utilx::variable_data_t& callingAddrInBinary = message.getCallingAddress();
  callingAddr.unpackOcts(callingAddrInBinary.data, callingAddrInBinary.dataLen);
  xudtMsg->setCallingAddress(callingAddr);

  const utilx::variable_data_t& userData = message.getUserData();
  xudtMsg->setData(userData.data, userData.dataLen);

  if ( message.isSetImportance() )
    xudtMsg->setImportance(message.getImportance());

  return *xudtMsg;
}

sccp::messages::XUDT&
MessageHandlers::formXUDT(const libsccp::N_UNITDATA_REQ_Message& message,
                          uint16_t next_data_part_start,
                          uint16_t next_data_part_sz,
                          uint8_t remaining_segments, uint32_t local_ref)
{
  utilx::alloc_mem_desc_t* mem_header =
      utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_SCCP_MESSAGE>();
  sccp::messages::XUDT* xudtMsg =  new (mem_header->allocated_memory) sccp::messages::XUDT();

  if ( message.getReturnOption() )
    xudtMsg->setProtocolClass(messages::ProtocolClass(messages::CLASS1_CONNECIONLESS,
                                                      messages::RETURN_MSG_ON_ERROR));
  else
    xudtMsg->setProtocolClass(messages::ProtocolClass(messages::CLASS1_CONNECIONLESS,
                                                      messages::NO_SPECIAL_OPTION));

  if ( message.isSetHopCounter() )
    xudtMsg->setHopCounter(message.getHopCounter());
  else
    xudtMsg->setHopCounter(MAX_HOP_COUNTER_VALUE);

  eyeline::sccp::SCCPAddress calledAddr;
  const utilx::variable_data_t& calledAddrInBinary = message.getCalledAddress();
  calledAddr.unpackOcts(calledAddrInBinary.data, calledAddrInBinary.dataLen);
  xudtMsg->setCalledAddress(calledAddr);

  eyeline::sccp::SCCPAddress callingAddr;
  const utilx::variable_data_t& callingAddrInBinary = message.getCallingAddress();
  callingAddr.unpackOcts(callingAddrInBinary.data, callingAddrInBinary.dataLen);
  xudtMsg->setCallingAddress(callingAddr);

  const utilx::variable_data_t& userData = message.getUserData();
  smsc_log_debug(_logger, "MessageHandlers::formXUDT::: next segment : next_data_part_start=%u, next_data_part_sz=%u",
                 next_data_part_start, next_data_part_sz);
  xudtMsg->setData(userData.data + next_data_part_start, next_data_part_sz);

  messages::Segmentation segmentation(next_data_part_start == 0,
                                      (message.isSetSequenceControl() ? messages::Segmentation::Class_1 :
                                                                        messages::Segmentation::Class_0),
                                      remaining_segments, local_ref);
  xudtMsg->setSegmentation(segmentation);

  if ( message.isSetImportance() )
    xudtMsg->setImportance(message.getImportance());

  return *xudtMsg;
}

sccp::messages::UDT&
MessageHandlers::formUDT(const libsccp::N_UNITDATA_REQ_Message& message)
{
  utilx::alloc_mem_desc_t* mem_header =
      utilx::PreallocatedMemoryManager::getInstance().getMemory<utilx::PreallocatedMemoryManager::MEM_FOR_SCCP_MESSAGE>();
  sccp::messages::UDT* udtMsg =  new (mem_header->allocated_memory) sccp::messages::UDT();

  messages::protocol_class_t protocolClass = messages::CLASS0_CONNECIONLESS;
  if ( message.isSetSequenceControl() )
    protocolClass = messages::CLASS1_CONNECIONLESS;

  if ( message.getReturnOption() )
    udtMsg->setProtocolClass(messages::ProtocolClass(protocolClass,
                                                     messages::RETURN_MSG_ON_ERROR));
  else
    udtMsg->setProtocolClass(messages::ProtocolClass(protocolClass,
                                                     messages::NO_SPECIAL_OPTION));

  eyeline::sccp::SCCPAddress calledAddr;
  const utilx::variable_data_t& calledAddrInBinary = message.getCalledAddress();
  calledAddr.unpackOcts(calledAddrInBinary.data, calledAddrInBinary.dataLen);
  smsc_log_debug(_logger, "MessageHandlers::formUDT::: set calledAddr=%s", calledAddr.toString().c_str());
  udtMsg->setCalledAddress(calledAddr);

  eyeline::sccp::SCCPAddress callingAddr;
  const utilx::variable_data_t& callingAddrInBinary = message.getCallingAddress();
  callingAddr.unpackOcts(callingAddrInBinary.data, callingAddrInBinary.dataLen);
  smsc_log_debug(_logger, "MessageHandlers::formUDT::: set callingAddr=%s", callingAddr.toString().c_str());
  udtMsg->setCallingAddress(callingAddr);

  const utilx::variable_data_t& userData = message.getUserData();
  udtMsg->setData(userData.data, userData.dataLen);

  return *udtMsg;
}

void
MessageHandlers::formNegativeResponse(const libsccp::N_UNITDATA_REQ_Message& message,
                                      common::return_cause_value_t return_cause,
                                      const common::LinkId& link_id)
{
  if ( message.getReturnOption() )
    _cMgr.send(link_id, libsccp::N_NOTICE_IND_Message(message, return_cause));
}

uint32_t
MessageHandlers::getUniqueLocaReference()
{
  smsc::core::synchronization::MutexGuard synchronize(_uniqRefLock);
  return ++_uniqueLocaReference;
}

smsc::core::synchronization::Mutex MessageHandlers::_uniqRefLock;
uint32_t MessageHandlers::_uniqueLocaReference;

MessageHandlers::authentication_res_e
MessageHandlers::makeAuthentication(const std::string& app_id)
{
  if ( ApplicationsRegistry::getInstance().hasElement(app_id) ) {
    smsc_log_info(_logger, "MessageHandlers::makeAuthentication::: application with id='%s' has been authenticated successfully",
                  app_id.c_str());
    return AUTHENTICATION_OK;
  } else {
    smsc_log_error(_logger, "MessageHandlers::makeAuthentication::: authentication failed for application with id='%s'",
                  app_id.c_str());
    return AUTHENTICATION_FAILED;
  }
}

}}}}}
