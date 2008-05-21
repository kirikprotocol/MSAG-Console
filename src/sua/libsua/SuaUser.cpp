#include "SuaUser.hpp"
#include "Exception.hpp"
#include <core/synchronization/MutexGuard.hpp>
#include <utility>
#include <sua/communication/TP.hpp>
#include <sua/communication/libsua_messages/BindMessage.hpp>
#include <sua/communication/libsua_messages/BindConfirmMessage.hpp>
#include <sua/communication/libsua_messages/UnbindMessage.hpp>
#include <sua/communication/libsua_messages/N_UNITDATA_REQ_Message.hpp>

extern std::string hexdmp(const uchar_t* buf, uint32_t bufSz);

namespace libsua {

SuaUser::SuaUser()
  : _wasInitialized(false), _logger(smsc::logger::Logger::getInstance("libsua")), _hopCountValue(0)
{}

void
SuaUser::sua_init(smsc::util::config::ConfigView* config)
{
  if ( !_wasInitialized ) {
    _appId = config->getString("appId", "SuaUser::sua_init::: appId parameter wasn't set");
    _trafficMode = config->getString("traffic-mode", "SuaUser::sua_init::: traffic-mode parameter wasn't set");
    try {
      _hopCountValue = config->getInt("ss7hop-count");
    } catch (smsc::util::config::ConfigException& ex) {
      _hopCountValue = 15;
    }
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

unsigned int
SuaUser::bind(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    return NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( suaConnectNum >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  if ( linkInfo.connectionState == CONNECTED ) {
    communication::TP tp;
    libsua_messages::BindMessage bindMessage;
    bindMessage.setAppId(_appId);
    bindMessage.serialize(&tp);

    smsc_log_info(_logger, "send Bind message=[%s] to link=[%s], linkId=%d", bindMessage.toString().c_str(), linkInfo.toString().c_str(), suaConnectNum);
    linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);

    corex::io::InputStream* iStream = linkInfo.socket->getInputStream();
    tp.packetLen = sizeof(uint32_t);
    uint32_t offset=0;
    do {
      offset += iStream->read(tp.packetBody + offset, tp.packetLen);
      printf("after first iStream->read offset=%d\n", offset);
      tp.packetLen -= offset;
    } while ( tp.packetLen > 0 );
    printf("after do{}while() offset=%d\n", offset);
    uint32_t tmpValue;
    memcpy(reinterpret_cast<uint8_t*>(&tmpValue), tp.packetBody, sizeof(tmpValue));
    tp.packetLen = ntohl(tmpValue);
    printf("got packetLen=%d\n", tp.packetLen);
    if ( tp.packetLen > communication::TP::MAX_PACKET_SIZE ) {
      smsc_log_error(_logger, "SuaUser::bind::: value of packetLen=[%d] excedeed max permited value=[%d]", tp.packetLen, communication::TP::MAX_PACKET_SIZE);
      return GOT_TOO_LONG_MESSAGE;
    }
    uint32_t numBytesToRead = tp.packetLen, bytesWasRead = 0;
    tp.packetLen += sizeof(uint32_t);
    do {
      printf("SuaUser::bind::: offset=%d,numBytesToRead=%d,tp.packetBody=%p,tp.packetBody+offset=%p\n", offset, numBytesToRead, tp.packetBody, tp.packetBody + offset);

      bytesWasRead = iStream->read(tp.packetBody + offset, numBytesToRead);
      numBytesToRead -= bytesWasRead;
    } while ( numBytesToRead > 0 );

    libsua_messages::BindConfirmMessage bindConfirmMessage;
    bindConfirmMessage.deserialize(tp);
    if ( bindConfirmMessage.getStatus() == libsua_messages::BindConfirmMessage::BIND_OK ) {
      linkInfo.connectionState = BINDED;
      _socketPool.insert(linkInfo.inputStream);
    }

    return bindConfirmMessage.getStatus();
  } else {
    smsc_log_error(_logger, "SuaUser::bind::: connection for link=[%s] hasn't been established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
    return NOT_CONNECTED;
  }
}

void
SuaUser::unbind(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    throw SuaLibException("SuaUser::sua_unbind::: not initialized");

  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( suaConnectNum >= _knownLinks.size() )
    return;

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  if ( linkInfo.connectionState == BINDED ) {
    communication::TP tp;
    libsua_messages::UnbindMessage unbindMessage;
    unbindMessage.serialize(&tp);

    smsc_log_info(_logger, "send Unbind message to link=[%s], linkId=%d", linkInfo.toString().c_str(), suaConnectNum);

    linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);
    linkInfo.connectionState = CONNECTED;
  }
}

unsigned int
SuaUser::unitdata_req(const uint8_t* message,
                      uint16_t messageSize,
                      const uint8_t* calledAddr,
                      uint8_t calledAddrLen,
                      const uint8_t* callingAddr,
                      uint8_t callingAddrLen,
                      const MessageProperties& msgProperties,
                      unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    return NOT_INITIALIZED;

  libsua_messages::N_UNITDATA_REQ_Message unitdataReqMessage;

  if ( msgProperties.fieldsMask & MessageProperties::SET_SEQUENCE_CONTROL )
    unitdataReqMessage.setSequenceControl(msgProperties.sequenceControlValue);

  unitdataReqMessage.setReturnOption(msgProperties.returnOnError);

  if ( msgProperties.fieldsMask & MessageProperties::SET_IMPORTANCE )
    unitdataReqMessage.setImportance(msgProperties.importance);

  if ( msgProperties.fieldsMask & MessageProperties::SET_HOP_COUNT )
    unitdataReqMessage.setHopCounter(msgProperties.hopCount);

  unitdataReqMessage.setCalledAddress(calledAddr, calledAddrLen);

  unitdataReqMessage.setCallingAddress(callingAddr, callingAddrLen);

  unitdataReqMessage.setUserData(message, messageSize);

  communication::TP tp;
  unitdataReqMessage.serialize(&tp);

  smsc::core::synchronization::MutexGuard synchronize(_lock);

  if ( suaConnectNum >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  if ( linkInfo.connectionState == BINDED ) {
    smsc_log_info(_logger, "send message=[%s] to link=[%s], linkId=%d", hexdmp(tp.packetBody, tp.packetLen).c_str(), linkInfo.toString().c_str(), suaConnectNum);
    linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);
  } else {
    smsc_log_error(_logger, "SuaUser::unitdata_req::: connection for link=[%s] hasn't been binded, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
    return NOT_BINDED;
  }
}

void
SuaUser::msgRecv(MessageInfo* msgInfo)
{
  if ( !_wasInitialized )
    throw SuaLibException("SuaUser::msgRecv::: not initialized");

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

      _packetsCache.erase(iter);
      return;
    }
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
