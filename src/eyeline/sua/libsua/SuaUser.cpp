#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <utility>
#include "core/synchronization/MutexGuard.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/toLowerCaseString.hpp"
#include "eyeline/sua/communication/TP.hpp"
#include "eyeline/sua/communication/libsua_messages/BindMessage.hpp"
#include "eyeline/sua/communication/libsua_messages/BindConfirmMessage.hpp"
#include "eyeline/sua/communication/libsua_messages/UnbindMessage.hpp"
#include "eyeline/sua/communication/libsua_messages/N_UNITDATA_REQ_Message.hpp"

#include "eyeline/sua/libsua/SuaUser.hpp"
#include "eyeline/sua/libsua/Exception.hpp"

namespace eyeline {
namespace sua {
namespace libsua {

SuaUser::SuaUser()
  : _wasInitialized(false), _logger(smsc::logger::Logger::getInstance("libsua")), _lastUsedConnIdx(0)
{
  availableData=0;
}

SuaApi::ErrorCode_e
SuaUser::sua_init(smsc::util::config::ConfigView* config)
{
  if ( !_wasInitialized ) {
    try {
      _appId = config->getString("appId", "SuaUser::sua_init::: appId parameter wasn't set");
      std::string trafficMode = config->getString("traffic-mode", "SuaUser::sua_init::: traffic-mode parameter wasn't set");
      _trafficMode = convertStringToTrafficModeValue(utilx::toLowerCaseString(trafficMode), "SuaUser::sua_init");
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
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "SuaUser::sua_init::: catched unexpected exception=[%s]", ex.what());
      return SYSTEM_MALFUNCTION;
    }
  }
  return OK;
}

SuaApi::ErrorCode_e
SuaUser::sua_close()
{
  try {
    while ( !_knownLinks.empty() ) {
      LinkInfo& linkInfo = _knownLinks[0];
      if ( linkInfo.inputStream )
        _socketPool.remove(linkInfo.inputStream);
      if ( linkInfo.socket ) {
        linkInfo.socket->close();
        delete linkInfo.socket;
      }
    }
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SuaUser::sua_close::: catched SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SuaUser::sua_close::: catched unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }

  return OK;
}

SuaApi::ErrorCode_e
SuaUser::sua_connect(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    return SUA_NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_lock);

  if ( suaConnectNum >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  smsc_log_info(_logger, "try establish connection for link=[%s], linkId=%d", linkInfo.toString().c_str(), suaConnectNum);

  try {
    if ( linkInfo.connectionState == linkNOT_CONNECTED ) {
      linkInfo.socket = new corex::io::network::TCPSocket(linkInfo.suaLayerHost, linkInfo.suaLayerPort);
      linkInfo.socket->connect();
      linkInfo.inputStream = new LinkInputStream(linkInfo.socket->getInputStream(), suaConnectNum);

      linkInfo.connectionState = linkCONNECTED;
      smsc_log_info(_logger, "connection for link=[%s] has been established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
    } else
      smsc_log_error(_logger, "connection [%s] has been already established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);

  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SuaUser::sua_connect::: catched SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SuaUser::sua_connect::: catched unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
  return OK;
}

SuaApi::ErrorCode_e
SuaUser::sua_disconnect(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    return SUA_NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_lock);

  if ( suaConnectNum >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  smsc_log_info(_logger, "try release connection for link=[%s], linkId=%d", linkInfo.toString().c_str(), suaConnectNum);

  try {
    if ( linkInfo.connectionState == linkCONNECTED ) {
      _socketPool.remove(linkInfo.inputStream);
      delete linkInfo.inputStream; linkInfo.inputStream = NULL;
      linkInfo.socket->close();
      linkInfo.connectionState = linkNOT_CONNECTED;
      delete linkInfo.socket; linkInfo.socket = NULL;
      smsc_log_info(_logger, "connection for link=[%s] has been released, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);

      return OK;
    } else {
      smsc_log_error(_logger, "connection [%s] hasn't been established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
      return NOT_CONNECTED;
    }
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SuaUser::sua_disconnect::: catched SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SuaUser::sua_disconnect::: catched unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
}

SuaApi::ErrorCode_e
SuaUser::bind(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    return SUA_NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( suaConnectNum >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  try {
    if ( linkInfo.connectionState == linkCONNECTED ) {
      communication::TP tp;
      communication::libsua_messages::BindMessage bindMessage;
      bindMessage.setAppId(_appId);
      bindMessage.serialize(&tp);

      smsc_log_info(_logger, "send Bind message=[%s] to link=[%s], linkId=%d", bindMessage.toString().c_str(), linkInfo.toString().c_str(), suaConnectNum);
      linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);

      corex::io::InputStream* iStream = linkInfo.socket->getInputStream();
      tp.packetLen = sizeof(uint32_t);
      size_t offset=0;
      do {
        offset += iStream->read(tp.packetBody + offset, tp.packetLen);
        tp.packetLen -= offset;
      } while ( tp.packetLen > 0 );
      uint32_t tmpValue;
      memcpy(reinterpret_cast<uint8_t*>(&tmpValue), tp.packetBody, sizeof(tmpValue));
      tp.packetLen = ntohl(tmpValue);
      if ( tp.packetLen > communication::TP::MAX_PACKET_SIZE ) {
        smsc_log_error(_logger, "SuaUser::bind::: value of packetLen=[%d] excedeed max permited value=[%d]", tp.packetLen, communication::TP::MAX_PACKET_SIZE);
        return GOT_TOO_LONG_MESSAGE;
      }
      size_t numBytesToRead = tp.packetLen, bytesWasRead = 0;
      tp.packetLen += sizeof(uint32_t);
      do {
        bytesWasRead = iStream->read(tp.packetBody + offset, numBytesToRead);
        numBytesToRead -= bytesWasRead;
      } while ( numBytesToRead > 0 );

      communication::libsua_messages::BindConfirmMessage bindConfirmMessage;
      bindConfirmMessage.deserialize(tp);
      if ( bindConfirmMessage.getStatus() == communication::libsua_messages::BindConfirmMessage::BIND_OK ) {
        linkInfo.connectionState = linkBINDED;
        _socketPool.insert(linkInfo.inputStream);
      }
      //NOTE: bind result related values of SuaApi::ErrorCode_e biuniquely conform
      //to corresponding BindConfirmMessage::BindResult_e values !!!
      return static_cast<SuaApi::ErrorCode_e>(bindConfirmMessage.getStatus());
    } else if ( linkInfo.connectionState == linkBINDED )
      return ALREADY_BINDED;
    else {
      smsc_log_error(_logger, "SuaUser::bind::: connection for link=[%s] hasn't been established, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
      return NOT_CONNECTED;
    }
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SuaUser::bind::: catched SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SuaUser::bind::: catched unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
}

SuaApi::ErrorCode_e
SuaUser::unbind(unsigned int suaConnectNum)
{
  if ( !_wasInitialized )
    return SUA_NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( suaConnectNum >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[suaConnectNum];

  try {
    if ( linkInfo.connectionState == linkBINDED ) {
      communication::TP tp;
      communication::libsua_messages::UnbindMessage unbindMessage;
      unbindMessage.serialize(&tp);

      smsc_log_info(_logger, "send Unbind message to link=[%s], linkId=%d", linkInfo.toString().c_str(), suaConnectNum);

      linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);
      linkInfo.connectionState = linkCONNECTED;

      return OK;
    } else
      return NOT_BINDED;
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SuaUser::unbind::: catched SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SuaUser::unbind::: catched unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
}

SuaApi::CallResult
SuaUser::unitdata_req(const uint8_t* message,
                      uint16_t messageSize,
                      const uint8_t* calledAddr,
                      uint8_t calledAddrLen,
                      const uint8_t* callingAddr,
                      uint8_t callingAddrLen,
                      const MessageProperties& msgProperties)
{
  unsigned int suaConnectNum = getConnNumByPolicy();

  return unitdata_req(message, messageSize, calledAddr, calledAddrLen,
                      callingAddr, callingAddrLen, msgProperties,
                      suaConnectNum);
}

SuaApi::CallResult
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
    return SuaApi::CallResult(SUA_NOT_INITIALIZED, suaConnectNum);

  communication::libsua_messages::N_UNITDATA_REQ_Message unitdataReqMessage;

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

  try {
    smsc::core::synchronization::MutexGuard synchronize(_lock);

    if ( suaConnectNum >= _knownLinks.size() )
      return SuaApi::CallResult(WRONG_CONNECT_NUM, suaConnectNum);

    LinkInfo& linkInfo = _knownLinks[suaConnectNum];

    if ( linkInfo.connectionState == linkBINDED ) {
      linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);
      smsc_log_info(_logger, "send message=[%s] to link=[%s], linkId=%d", utilx::hexdmp(tp.packetBody, tp.packetLen).c_str(), linkInfo.toString().c_str(), suaConnectNum);
      return SuaApi::CallResult(OK, suaConnectNum);
    } else {
      smsc_log_error(_logger, "SuaUser::unitdata_req::: connection for link=[%s] hasn't been binded, linkId=%d", linkInfo.toString().c_str(), suaConnectNum);
      return SuaApi::CallResult(NOT_BINDED, suaConnectNum);
    }
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SuaUser::unitdata_req::: catched SystemError exception=[%s]", ex.what());
    return SuaApi::CallResult(SYSTEM_ERROR, suaConnectNum);
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SuaUser::unitdata_req::: catched unexpected exception=[%s]", ex.what());
    return SuaApi::CallResult(SYSTEM_MALFUNCTION, suaConnectNum);
  }
}

SuaApi::ErrorCode_e
SuaUser::msgRecv(MessageInfo* msgInfo, uint32_t timeout)
{
  if ( !_wasInitialized )
    return SUA_NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_receiveSynchronizeLock);

  try {
    if ( timeout ) {
      int res = _socketPool.listen(timeout);
      if ( res == corex::io::IOObjectsPool::TIMEOUT )
        return SOCKET_TIMEOUT;
    } else
      _socketPool.listen();

    corex::io::InputStream* iStream=0;
    while ( availableData || (iStream = _socketPool.getNextReadyInputStream()) ) {
      if(availableData)
      {
        iStream=availableData->owner;
      }
      packets_cache_t::iterator iter = _packetsCache.find(iStream);
      if ( iter == _packetsCache.end() )
      {
        std::pair<packets_cache_t::iterator,bool> ins_res =  _packetsCache.insert(std::make_pair(iStream, new CacheEntry()));
        iter = ins_res.first;
      }

      CacheEntry* cacheEntry = availableData?availableData:iter->second;
      cacheEntry->owner=iStream;

      if ( cacheEntry->expectedMessageSize == 0 || 
           cacheEntry->expectedMessageSize > cacheEntry->ringBuf.getSizeOfAvailData() )
        cacheEntry->ringBuf.load(iStream);

      if ( cacheEntry->expectedMessageSize == 0 && cacheEntry->ringBuf.getSizeOfAvailData()>=4)
        cacheEntry->expectedMessageSize = cacheEntry->ringBuf.readUint32();

      smsc_log_debug(_logger,"msgRecv: cacheEntry.expectedMessageSize=%d, cacheEntry.getSizeOfAvailData()=%d",cacheEntry->expectedMessageSize,cacheEntry->ringBuf.getSizeOfAvailData());

      if ( cacheEntry->expectedMessageSize!=0 && cacheEntry->expectedMessageSize <= cacheEntry->ringBuf.getSizeOfAvailData() ) {
        uint32_t lenField = htonl(cacheEntry->expectedMessageSize);

        msgInfo->msgData.reset(cacheEntry->expectedMessageSize + sizeof(cacheEntry->expectedMessageSize));
        msgInfo->messageType = cacheEntry->ringBuf.readUint32();

        uint32_t msgTypeField = htonl(msgInfo->messageType);

        msgInfo->msgData.Append(reinterpret_cast<uint8_t*>(&lenField), sizeof(lenField));
//        memcpy(msgInfo->msgData.getCurPtr(), reinterpret_cast<uint8_t*>(&lenField), sizeof(lenField));
//        msgInfo->msgData.setPos(msgInfo->msgData.getPos() + sizeof(lenField));
        msgInfo->msgData.Append(reinterpret_cast<uint8_t*>(&msgTypeField), sizeof(msgTypeField));
//        memcpy(msgInfo->msgData.getCurPtr(), reinterpret_cast<uint8_t*>(&msgTypeField), sizeof(msgTypeField));
//        msgInfo->msgData.setPos(msgInfo->msgData.getPos() + sizeof(msgTypeField));

        cacheEntry->ringBuf.readArray(msgInfo->msgData.getCurPtr(), cacheEntry->expectedMessageSize - sizeof(msgInfo->messageType));
        msgInfo->msgData.setPos(msgInfo->msgData.getPos() + cacheEntry->expectedMessageSize - sizeof(msgInfo->messageType));
        msgInfo->suaConnectNum = static_cast<LinkInputStream*>(iStream)->getConnectNum();

        //delete cacheEntry;
        //_packetsCache.erase(iter);
        smsc_log_debug(_logger,"msgRecv: after cacheEntry.expectedMessageSize=%d, cacheEntry.getSizeOfAvailData()=%d",cacheEntry->expectedMessageSize,cacheEntry->ringBuf.getSizeOfAvailData());
	cacheEntry->expectedMessageSize=0;
	if(cacheEntry->ringBuf.getSizeOfAvailData()>=4)
	{
	  cacheEntry->expectedMessageSize=cacheEntry->ringBuf.readUint32();
	  if(cacheEntry->expectedMessageSize<=cacheEntry->ringBuf.getSizeOfAvailData())
	  {
	    availableData=cacheEntry;
          }else
	  {
            availableData=0;
	  }
	}else
	{
	  availableData=0;
	}
        return OK;
      }
    }

    return OK;
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SuaUser::msgRecv::: catched SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SuaUser::msgRecv::: catched unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
}


size_t
SuaUser::sua_getConnectsCount() const
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);
  return _knownLinks.size();
}

int
SuaUser::getConnNumByPolicy()
{
  if ( _trafficMode == OVERRIDE) {
    return _lastUsedConnIdx;
  } else if ( _trafficMode = LOADSHARE ) {
    smsc::core::synchronization::MutexGuard synchronize(_lastUsedConnIdxLock);
    _lastUsedConnIdx = (_lastUsedConnIdx + 1) % _knownLinks.size();
    return _lastUsedConnIdx;
  } else
    throw SuaLibException("SuaUser::getConnNumByPolicy::: invalid traffic mode=[%d]", _trafficMode);
}

SuaUser::LinkInfo::LinkInfo()
  : suaLayerPort(0),
    socket(NULL), connectionState(linkNOT_CONNECTED), inputStream(NULL) {}

SuaUser::LinkInfo::LinkInfo(const std::string& aLinkName, const std::string& aSuaLayerHost, in_port_t aSuaLayerPort)
  : linkName(aLinkName), suaLayerHost(aSuaLayerHost), suaLayerPort(aSuaLayerPort),
    socket(NULL), connectionState(linkNOT_CONNECTED), inputStream(NULL)
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

SuaUser::traffic_mode_t
SuaUser::convertStringToTrafficModeValue(const std::string& trafficMode, const std::string& where)
{
  if ( trafficMode == "loadshare" )
    return LOADSHARE;
  else if ( trafficMode == "override" )
    return OVERRIDE;
  else {
    const std::string fmtStr = where + "convertStringToTrafficModeValue::: wrong traffic-mode parameter value=[%s]";
    throw smsc::util::Exception(fmtStr.c_str(), trafficMode.c_str());
  }
}

}}}
