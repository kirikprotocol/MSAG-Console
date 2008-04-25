#include "SuaUser.hpp"
#include "Exception.hpp"
#include <core/synchronization/MutexGuard.hpp>
#include <utility>
#include <sua/communication/TP.hpp>
#include <sua/communication/libsua_messages/BindMessage.hpp>
#include <sua/communication/libsua_messages/EncapsulatedSuaMessage.hpp>
#include <sua/communication/libsua_messages/BindMessage.hpp>
#include <sua/communication/libsua_messages/UnbindMessage.hpp>
#include <sua/communication/sua_messages/CLDTMessage.hpp>
#include <sua/communication/sua_messages/SCONMessage.hpp>

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace libsua {

SuaUser::SuaUser()
  : _wasInitialized(false), _logger(smsc::logger::Logger::getInstance("libsua"))
{}

void
SuaUser::sua_init(smsc::util::config::ConfigView* config)
{
  if ( !_wasInitialized ) {
    _appId = config->getString("appId", "SuaUser::sua_init::: appId parameter wasn't set");
    _trafficMode = config->getString("traffic-mode", "SuaUser::sua_init::: traffic-mode parameter wasn't set");

    smsc_log_info(_logger, "loading links configuration ...");

    std::auto_ptr< std::set<std::string> > setGuard(config->getShortSectionNames());

    unsigned int linkIdx=0;
    for (std::set<std::string>::iterator i=setGuard->begin(), end_iter = setGuard->end();
         i!=end_iter;i++)
    {
      try
      {
        const std::string& linkName = *i;
        smsc_log_info(_logger, "loading link '%s' ...", linkName.c_str());

        std::auto_ptr<smsc::util::config::ConfigView> linkConfigGuard(config->getSubConfig(linkName.c_str()));

        LinkInfo linkInfo(linkName,
                          linkConfigGuard->getString("suaLayerHost", "suaLayerHost parameter wasn't set"),
                          linkConfigGuard->getInt("suaLayerPort", "suaLayerPort parameter wasn't set"));

        _knownLinks.push_back(linkInfo);
        smsc_log_info(_logger, "registered link=[%s], linkId=%d", linkInfo.toString().c_str(), linkIdx);
        ++linkIdx;
      } catch (smsc::util::config::ConfigException& ce) {}
    }
    _wasInitialized = true;
  }
}

void
SuaUser::sua_close()
{}

void
SuaUser::sua_connect(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    throw SuaLibException("SuaUser::sua_connect::: not initialized");

  smsc::core::synchronization::MutexGuard synchronize(_lock);

  if ( suaConnectNum >= _knownLinks.size() )
    throw SuaLibException("SuaUser::sua_connect::: wrong connectNum value=[%d]", suaConnectNum);

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  smsc_log_info(_logger, "try establish connection for link=[%s], linkId=%d", linkInfo.toString().c_str(), suaConnectNum);

  if ( linkInfo.connectionState == NOT_CONNECTED ) {
    linkInfo.socket = new corex::io::network::TCPSocket(linkInfo.suaLayerHost, linkInfo.suaLayerPort);
    linkInfo.socket->connect();
    linkInfo.inputStream = new LinkInputStream(linkInfo.socket->getInputStream(), suaConnectNum);

    linkInfo.connectionState = CONNECTED;
    smsc_log_info(_logger, "connection for link=[%s] has been established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
  } else
    smsc_log_error(_logger, "connection [%s] has been already established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
}

void
SuaUser::sua_disconnect(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    throw SuaLibException("SuaUser::sua_disconnect::: not initialized");

  smsc::core::synchronization::MutexGuard synchronize(_lock);

  if ( suaConnectNum >= _knownLinks.size() )
    throw SuaLibException("SuaUser::sua_disconnect::: wrong connectNum value=[%d]", suaConnectNum);

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  smsc_log_info(_logger, "try release connection for link=[%s], linkId=%d", linkInfo.toString().c_str(), suaConnectNum);

  if ( linkInfo.connectionState == CONNECTED ) {
    _socketPool.remove(linkInfo.inputStream);
    delete linkInfo.inputStream; linkInfo.inputStream = NULL;
    linkInfo.socket->close();
    linkInfo.connectionState = NOT_CONNECTED;
    delete linkInfo.socket; linkInfo.socket = NULL;
    smsc_log_info(_logger, "connection for link=[%s] has been released, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
  } else
    smsc_log_error(_logger, "connection [%s] hasn't been established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
}

void
SuaUser::sua_bind(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    throw SuaLibException("SuaUser::sua_bind::: not initialized");

  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( suaConnectNum >= _knownLinks.size() )
    throw SuaLibException("SuaUser::sua_bind::: wrong connectNum value=[%d]", suaConnectNum);

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  if ( linkInfo.connectionState == CONNECTED ) {
    communication::TP tp;
    libsua_messages::BindMessage bindMessage;
    bindMessage.setAppId(_appId);
    bindMessage.serialize(&tp);

    smsc_log_info(_logger, "send Bind message=[%s] to link=[%s], linkId=%d", bindMessage.toString().c_str(), linkInfo.toString().c_str(), suaConnectNum);
    linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);
    linkInfo.connectionState = BINDED;
    _socketPool.insert(linkInfo.inputStream);
  } else {
    smsc_log_error(_logger, "SuaUser::sua_bind::: connection for link=[%s] hasn't been established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
    throw SuaLibException("SuaUser::sua_bind:: not connected");
  }
}

void
SuaUser::sua_unbind(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    throw SuaLibException("SuaUser::sua_unbind::: not initialized");

  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( suaConnectNum >= _knownLinks.size() )
    throw SuaLibException("SuaUser::sua_unbind::: wrong connectNum value=[%d]", suaConnectNum);

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  if ( linkInfo.connectionState == BINDED ) {
    communication::TP tp;
    libsua_messages::UnbindMessage unbindMessage;
    unbindMessage.serialize(&tp);

    smsc_log_info(_logger, "send Unbind message to link=[%s], linkId=%d", linkInfo.toString().c_str(), suaConnectNum);

    linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);
    linkInfo.connectionState = CONNECTED;
  } else {
    smsc_log_error(_logger, "SuaUser::sua_unbind::: connection for link=[%s] hasn't been established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
    throw SuaLibException("SuaUser::sua_unbind:: not connected");
  }
}

void
SuaUser::sua_send_cldt(const uint8_t* message,
                       uint16_t messageSize,
                       const SCCPAddress& srcAddress,
                       const SCCPAddress& dstAddress,
                       const MessageProperties& msgProperties,
                       unsigned int suaConnectNum)
{
  sua_messages::CLDTMessage cldtMessage;
  uint32_t fakeIndexes[1] = {0};
  cldtMessage.setRoutingContext(sua_messages::TLV_RoutingContext(fakeIndexes, 1));

  if ( msgProperties.fieldsMask & MessageProperties::SET_PROT_CLASS )
    cldtMessage.setProtocolClass(sua_messages::TLV_ProtocolClass(sua_messages::ProtocolClass(sua_messages::protocol_class_t(msgProperties.protocolClass), sua_messages::ret_on_err_ind_t(msgProperties.returnOnError))));
  else
    throw smsc::util::Exception("SuaUser::sua_send_cldt::: protocolClass wasn't set");

  if ( srcAddress.fieldsMask & SCCPAddress::SET_SSN )
    cldtMessage.setSourceAddress(sua_messages::TLV_SourceAddress(sua_messages::TLV_GlobalTitle(srcAddress.gt), sua_messages::TLV_SSN(srcAddress.ssn)));
  else
    cldtMessage.setSourceAddress(sua_messages::TLV_SourceAddress(sua_messages::TLV_GlobalTitle(srcAddress.gt)));

  if ( dstAddress.fieldsMask & SCCPAddress::SET_SSN )
    cldtMessage.setDestinationAddress(sua_messages::TLV_DestinationAddress(sua_messages::TLV_GlobalTitle(dstAddress.gt), sua_messages::TLV_SSN(dstAddress.ssn)));
  else
    cldtMessage.setDestinationAddress(sua_messages::TLV_DestinationAddress(sua_messages::TLV_GlobalTitle(dstAddress.gt)));

  if ( msgProperties.fieldsMask & MessageProperties::SET_SEQUENCE_CONTROL )
    cldtMessage.setSequenceControl(sua_messages::TLV_SequenceControl(msgProperties.sequenceControlValue));
  else
    throw smsc::util::Exception("SuaUser::sua_send_cldt::: sequenceControl wasn't set");

  cldtMessage.setData(sua_messages::TLV_Data(message, messageSize));

  // next parameters is optional
  if ( msgProperties.fieldsMask & MessageProperties::SET_HOP_COUNT )
    cldtMessage.setSS7HopCount(sua_messages::TLV_SS7HopCount(msgProperties.hopCount));
  
  if ( msgProperties.fieldsMask & MessageProperties::SET_IMPORTANCE )
    cldtMessage.setImportance(sua_messages::TLV_Importance(msgProperties.importance));

  if ( msgProperties.fieldsMask & MessageProperties::SET_MESSAGE_PRIORITY )
    cldtMessage.setMessagePriority(sua_messages::TLV_MessagePriority(msgProperties.messagePriority));

  if ( msgProperties.fieldsMask & MessageProperties::SET_CORRELATION_ID )
    cldtMessage.setCorrelationId(sua_messages::TLV_CorrelationId(msgProperties.correlationId));

  if ( msgProperties.fieldsMask & MessageProperties::SET_SEGMENTATION )
    cldtMessage.setSegmentation(sua_messages::TLV_Segmentation(msgProperties.segmentation.firstSegment, msgProperties.segmentation.remainSegments, msgProperties.segmentation.segmentationReference));

  libsua_messages::EncapsulatedSuaMessage encapsulatedSuaMsg(&cldtMessage);

  communication::TP tp;
  encapsulatedSuaMsg.serialize(&tp);
  sua_sendmsg(tp.packetBody, tp.packetLen, suaConnectNum);
}

void
SuaUser::sua_send_scon(const sua_messages::PointCode& pc,
                       uint8_t congestionLevel,
                       unsigned int suaConnectNum)
{
  sua_messages::SCONMessage sconMessage;
  sconMessage.setAffectedPointCode(sua_messages::TLV_AffectedPointCode(pc));
  sconMessage.setCongestionLevel(sua_messages::TLV_CongestionLevel(congestionLevel));

  libsua_messages::EncapsulatedSuaMessage encapsulatedSuaMsg(&sconMessage);

  communication::TP tp;
  encapsulatedSuaMsg.serialize(&tp);
  sua_sendmsg(tp.packetBody, tp.packetLen, suaConnectNum);
}

void
SuaUser::sua_sendmsg(const uint8_t* message,
                     size_t messageSize,
                     unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    throw SuaLibException("SuaUser::sua_sendmsg::: not initialized");

  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( suaConnectNum >= _knownLinks.size() )
    throw SuaLibException("SuaUser::sua_sendmsg::: wrong connectNum value=[%d]", suaConnectNum);

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  if ( linkInfo.connectionState == BINDED ) {
    smsc_log_info(_logger, "send message=[%s] to link=[%s], linkId=%d", hexdmp(message, messageSize).c_str(), linkInfo.toString().c_str(), suaConnectNum);
    linkInfo.socket->getOutputStream()->write(message, messageSize);
  } else {
    smsc_log_error(_logger, "SuaUser::sua_sendmsg::: connection for link=[%s] hasn't been binded, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
    throw SuaLibException("SuaUser::sua_sendmsg:: not connected");
  }
}

void
SuaUser::sua_recvmsg(MessageInfo* msgInfo)
{
  if ( !_wasInitialized )
    throw SuaLibException("SuaUser::sua_recvmsg::: not initialized");

  smsc::core::synchronization::MutexGuard synchronize(_receiveSynchronizeLock);

  int res = _socketPool.listen();

  corex::io::InputStream* iStream;
  while ( iStream = _socketPool.getNextReadyInputStream() ) {
    packets_cache_t::iterator iter = _packetsCache.find(iStream);
    if ( iter == _packetsCache.end() ) {
      std::pair<packets_cache_t::iterator,bool> ins_res =  _packetsCache.insert(std::make_pair(iStream, CacheEntry()));
      iter = ins_res.first;
    }

    CacheEntry& cacheEntry = iter->second;

    if ( cacheEntry.expectedMessageSize == 0 || 
         cacheEntry.expectedMessageSize > cacheEntry.ringBuf->getSizeOfAvailData() )
      cacheEntry.ringBuf->load(iStream);

    if ( cacheEntry.expectedMessageSize == 0 )
      cacheEntry.expectedMessageSize = cacheEntry.ringBuf->readUint32();

    if ( cacheEntry.expectedMessageSize <= cacheEntry.ringBuf->getSizeOfAvailData() ) {
      msgInfo->msgData.setSize(cacheEntry.expectedMessageSize);
      msgInfo->messageType = cacheEntry.ringBuf->readUint32();
      cacheEntry.ringBuf->readArray(msgInfo->msgData.GetCurPtr(), cacheEntry.expectedMessageSize);
      msgInfo->msgData.SetPos(msgInfo->msgData.GetPos() + cacheEntry.expectedMessageSize);
      msgInfo->suaConnectNum = static_cast<LinkInputStream*>(iStream)->getConnectNum();
      fillUpMessageProperties(&msgInfo->msgProperties, msgInfo->messageType, msgInfo->msgData);

      _packetsCache.erase(iter);
      return;
    }
  }
}

void
SuaUser::fillUpMessageProperties(MessageProperties* msgProperties, uint32_t messageType, MessageInfo::msg_buffer_t& msgData)
{
  if ( messageType == libsua_messages::EncapsulatedSuaMessage::ENCAPSULATED_SUA_CLDT_MESSAGE_CODE ) {
    sua_messages::CLDTMessage cldtMsg;
    communication::TP tp;
    tp.packetType = sua_messages::SUAMessage::getMessageIndex(cldtMsg.getMsgCode());
    tp.packetLen = sua_messages::SUAMessage::makeHeader(tp.packetBody, cldtMsg.getMsgCode(), msgData.getSize());
    memcpy(tp.packetBody + tp.packetLen, msgData.get(), msgData.getSize());
    tp.packetLen += msgData.getSize();
    cldtMsg.deserialize(tp);

    msgProperties->protocolClass;
    msgProperties->returnOnError;
    msgProperties->sequenceControlValue;
    msgProperties->fieldsMask = MessageProperties::SET_PROT_CLASS | MessageProperties::SET_RETURN_ON_ERROR | MessageProperties::SET_SEQUENCE_CONTROL;

    try {
      const sua_messages::TLV_Importance& importance = cldtMsg.getImportance();
      msgProperties->importance = importance.getImportanceValue();
      msgProperties->fieldsMask |= MessageProperties::SET_IMPORTANCE;
    } catch(utilx::FieldNotSetException& ex) {}

    try {
      const sua_messages::TLV_MessagePriority& msgPrio = cldtMsg.getMessagePriority();
      msgProperties->messagePriority = msgPrio.getMessagePriorityValue();
      msgProperties->fieldsMask |= MessageProperties::SET_MESSAGE_PRIORITY;
    } catch(utilx::FieldNotSetException& ex) {}

    try {
      const sua_messages::TLV_CorrelationId& correlationId = cldtMsg.getCorrelationId();
      msgProperties->correlationId = correlationId.getValue();
      msgProperties->fieldsMask |= MessageProperties::SET_CORRELATION_ID;
    } catch(utilx::FieldNotSetException& ex) {}

    try {
      const sua_messages::TLV_Segmentation& segmentation = cldtMsg.getSegmentation();
      msgProperties->segmentation = Segmentation(segmentation.getReferenceValue(), segmentation.isFirstSegment(), segmentation.getNumOfRemainingSegments());
      msgProperties->fieldsMask |= MessageProperties::SET_SEGMENTATION;
    } catch(utilx::FieldNotSetException& ex) {}

    try {
      const sua_messages::TLV_SS7HopCount& hopCount = cldtMsg.getSS7HopCount();
      msgProperties->hopCount = hopCount.getHopCountValue();
      msgProperties->fieldsMask |= MessageProperties::SET_HOP_COUNT;
    } catch(utilx::FieldNotSetException& ex) {}

  }
}

unsigned int
SuaUser::sua_getConnectsCount() const
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);
  return _knownLinks.size();
}

unsigned int
SuaUser::getConnNumByPolicy()
{
  return 0;
}

SuaUser::LinkInfo::LinkInfo() : suaLayerPort(0) {}

SuaUser::LinkInfo::LinkInfo(const std::string& aLinkName, const std::string& aSuaLayerHost, in_port_t aSuaLayerPort)
  : linkName(aLinkName), suaLayerHost(aSuaLayerHost), suaLayerPort(aSuaLayerPort),
    socket(NULL), connectionState(NOT_CONNECTED), inputStream(NULL)
{
  if ( aLinkName == "" ) throw SuaLibException("LinkInfo::LinkInfo::: empty link name value");
  if ( aSuaLayerPort > 0xFFFF ) throw SuaLibException("LinkInfo::LinkInfo::: wrong port value=[%d]", aSuaLayerPort);
  if ( aSuaLayerHost == "" ) throw SuaLibException("LinkInfo::LinkInfo::: empty host value");
}

std::string
SuaUser::LinkInfo::toString() const
{
  char strBuf[256];
  snprintf(strBuf, sizeof(strBuf), "link=[%s],suaLayerHost=[%s],suaLayerPort=[%d]",linkName.c_str(), suaLayerHost.c_str(), suaLayerPort);
  return std::string(strBuf);
}

SuaUser::LinkInputStream::LinkInputStream(corex::io::InputStream* iStream, unsigned int connectNum)
  : _iStream(iStream), _connectNum(connectNum) {}

ssize_t
SuaUser::LinkInputStream::read(uint8_t *buf, size_t bufSz)
{
  return _iStream->read(buf, bufSz);
}

corex::io::IOObject*
SuaUser::LinkInputStream::getOwner()
{
  return _iStream->getOwner();
}

unsigned int
SuaUser::LinkInputStream::getConnectNum() const
{
  return _connectNum;
}

}
