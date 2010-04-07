#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <utility>
#include "core/synchronization/MutexGuard.hpp"
#include "eyeline/utilx/hexdmp.hpp"
#include "eyeline/utilx/toLowerCaseString.hpp"
#include "eyeline/ss7na/common/TP.hpp"
#include "eyeline/ss7na/libsccp/messages/BindMessage.hpp"
#include "eyeline/ss7na/libsccp/messages/BindConfirmMessage.hpp"
#include "eyeline/ss7na/libsccp/messages/UnbindMessage.hpp"
#include "eyeline/ss7na/libsccp/messages/N_UNITDATA_REQ_Message.hpp"

#include "eyeline/ss7na/libsccp/SccpUser.hpp"
#include "eyeline/ss7na/libsccp/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

SccpUser::SccpUser()
  : _wasInitialized(false), _logger(smsc::logger::Logger::getInstance("libsua")),
    _lastUsedConnIdx(0), _socketPool(MAX_SOCKET_POOL_SIZE)
{
  availableData=0;
}

SccpApi::ErrorCode_e
SccpUser::init(smsc::util::config::ConfigView* config)
{
  if ( !_wasInitialized ) {
    try {
      _appId = config->getString("appId", "SccpUser::init::: appId parameter wasn't set");
      std::string trafficMode = config->getString("traffic-mode", "SccpUser::init::: traffic-mode parameter wasn't set");
      _trafficMode = convertStringToTrafficModeValue(utilx::toLowerCaseString(trafficMode), "SccpUser::init");
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
                            linkConfigGuard->getString("host", "host parameter wasn't set"),
                            linkConfigGuard->getInt("port", "port parameter wasn't set"));

          _knownLinks.push_back(linkInfo);
          smsc_log_info(_logger, "registered link=[%s], linkId=%d", linkInfo.toString().c_str(), linkIdx);
          ++linkIdx;
        } catch (smsc::util::config::ConfigException& ce) {}
      }
      _wasInitialized = true;
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "SccpUser::init::: caught unexpected exception=[%s]", ex.what());
      return SYSTEM_MALFUNCTION;
    }
  }
  return OK;
}

SccpApi::ErrorCode_e
SccpUser::close()
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
    smsc_log_error(_logger, "SccpUser::close::: caught SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SccpUser::close::: caught unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }

  return OK;
}

SccpApi::ErrorCode_e
SccpUser::connect(unsigned int connect_num)
{
  if ( !_wasInitialized )
    return LIB_NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_lock);

  if ( connect_num >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[connect_num];

  smsc_log_info(_logger, "try establish connection for link=[%s], linkId=%d", linkInfo.toString().c_str(), connect_num);

  try {
    if ( linkInfo.connectionState == linkNOT_CONNECTED ) {
      linkInfo.socket = new corex::io::network::TCPSocket(linkInfo.host, linkInfo.port);
      linkInfo.socket->connect();
      linkInfo.inputStream = new LinkInputStream(linkInfo.socket->getInputStream(), connect_num);

      linkInfo.connectionState = linkCONNECTED;
      smsc_log_info(_logger, "connection for link=[%s] has been established, linkId=%d", linkInfo.toString().c_str(), connect_num);
    } else
      smsc_log_error(_logger, "connection [%s] has been already established, linkId=%d", linkInfo.toString().c_str(), connect_num);

  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SccpUser::connect::: caught SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SccpUser::connect::: caught unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
  return OK;
}

SccpApi::ErrorCode_e
SccpUser::disconnect(unsigned int connect_num)
{
  if ( !_wasInitialized )
    return LIB_NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_lock);

  if ( connect_num >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[connect_num];

  smsc_log_info(_logger, "try release connection for link=[%s], linkId=%d", linkInfo.toString().c_str(), connect_num);

  try {
    if ( linkInfo.connectionState == linkCONNECTED ) {
      _socketPool.remove(linkInfo.inputStream);
      delete linkInfo.inputStream; linkInfo.inputStream = NULL;
      linkInfo.socket->close();
      linkInfo.connectionState = linkNOT_CONNECTED;
      delete linkInfo.socket; linkInfo.socket = NULL;
      smsc_log_info(_logger, "connection for link=[%s] has been released, linkId=%d", linkInfo.toString().c_str(), connect_num);

      return OK;
    } else {
      smsc_log_error(_logger, "connection [%s] hasn't been established, linkId=%d", linkInfo.toString().c_str(), connect_num);
      return NOT_CONNECTED;
    }
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SccpUser::disconnect::: caught SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SccpUser::disconnect::: caught unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
}

SccpApi::ErrorCode_e
SccpUser::bind(unsigned int connect_num, uint8_t* ssn_list, uint8_t ssn_list_sz)
{
  if ( !_wasInitialized )
    return LIB_NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( connect_num >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[connect_num];

  try {
    if ( linkInfo.connectionState == linkCONNECTED ) {
      common::TP tp;
      BindMessage bindMessage;
      bindMessage.setAppId(_appId);
      bindMessage.setSSN(ssn_list, ssn_list_sz);
      bindMessage.serialize(&tp);

      smsc_log_info(_logger, "send Bind message=[%s] to link=[%s], linkId=%d", bindMessage.toString().c_str(), linkInfo.toString().c_str(), connect_num);
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
      if ( tp.packetLen > common::TP::MAX_PACKET_SIZE ) {
        smsc_log_error(_logger, "SccpUser::bind::: value of packetLen=[%d] excedeed max permited value=[%d]", tp.packetLen, common::TP::MAX_PACKET_SIZE);
        return GOT_TOO_LONG_MESSAGE;
      }
      size_t numBytesToRead = tp.packetLen, bytesWasRead = 0;
      tp.packetLen += sizeof(uint32_t);
      do {
        bytesWasRead = iStream->read(tp.packetBody + offset, numBytesToRead);
        numBytesToRead -= bytesWasRead;
      } while ( numBytesToRead > 0 );

      BindConfirmMessage bindConfirmMessage;
      bindConfirmMessage.deserialize(tp);
      if ( bindConfirmMessage.getStatus() == BindConfirmMessage::BIND_OK ) {
        linkInfo.connectionState = linkBINDED;
        _socketPool.insert(linkInfo.inputStream);
      }
      //NOTE: bind result related values of SuaApi::ErrorCode_e biuniquely conform
      //to corresponding BindConfirmMessage::BindResult_e values !!!
      return static_cast<ErrorCode_e>(bindConfirmMessage.getStatus());
    } else if ( linkInfo.connectionState == linkBINDED )
      return ALREADY_BINDED;
    else {
      smsc_log_error(_logger, "SccpUser::bind::: connection for link=[%s] hasn't been established, linkId=%d", linkInfo.toString().c_str(), connect_num);
      return NOT_CONNECTED;
    }
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SccpUser::bind::: caught SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SccpUser::bind::: caught unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
}

SccpApi::ErrorCode_e
SccpUser::unbind(unsigned int connect_num)
{
  if ( !_wasInitialized )
    return LIB_NOT_INITIALIZED;

  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( connect_num >= _knownLinks.size() )
    return WRONG_CONNECT_NUM;

  LinkInfo& linkInfo = _knownLinks[connect_num];

  try {
    if ( linkInfo.connectionState == linkBINDED ) {
      common::TP tp;
      UnbindMessage unbindMessage;
      unbindMessage.serialize(&tp);

      smsc_log_info(_logger, "send Unbind message to link=[%s], linkId=%d", linkInfo.toString().c_str(), connect_num);

      linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);
      linkInfo.connectionState = linkCONNECTED;

      return OK;
    } else
      return NOT_BINDED;
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SccpUser::unbind::: caught SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SccpUser::unbind::: caught unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
}

SccpApi::CallResult
SccpUser::unitdata_req(const uint8_t* message,
                      uint16_t messageSize,
                      const uint8_t* called_addr,
                      uint8_t called_addr_len,
                      const uint8_t* calling_addr,
                      uint8_t calling_addr_len,
                      const MessageProperties& msg_properties)
{
  unsigned int connectNum = getConnNumByPolicy();

  return unitdata_req(message, messageSize, called_addr, called_addr_len,
                      calling_addr, calling_addr_len, msg_properties,
                      connectNum);
}

SccpApi::CallResult
SccpUser::unitdata_req(const uint8_t* message,
                      uint16_t message_size,
                      const uint8_t* called_addr,
                      uint8_t called_addr_len,
                      const uint8_t* calling_addr,
                      uint8_t calling_addr_len,
                      const MessageProperties& msg_properties,
                      unsigned int connect_num)
{
  //TODO: avoid excessive memcpy() in serialization
  if ( !_wasInitialized )
    return CallResult(LIB_NOT_INITIALIZED, connect_num);

  N_UNITDATA_REQ_Message unitdataReqMessage;

  if (msg_properties.hasSequenceControl())
    unitdataReqMessage.setSequenceControl(msg_properties.getSequenceControl());

  unitdataReqMessage.setReturnOption(msg_properties.getReturnOnError());

  if (msg_properties.hasImportance())
    unitdataReqMessage.setImportance(msg_properties.getImportance());

  if (msg_properties.hasHopCount())
    unitdataReqMessage.setHopCounter(msg_properties.getHopCount());

  unitdataReqMessage.setCalledAddress(called_addr, called_addr_len);

  unitdataReqMessage.setCallingAddress(calling_addr, calling_addr_len);

  unitdataReqMessage.setUserData(message, message_size);

  common::TP tp;
  unitdataReqMessage.serialize(&tp);

  try {
    smsc::core::synchronization::MutexGuard synchronize(_lock);

    if ( connect_num >= _knownLinks.size() )
      return CallResult(WRONG_CONNECT_NUM, connect_num);

    LinkInfo& linkInfo = _knownLinks[connect_num];

    if ( linkInfo.connectionState == linkBINDED ) {
      linkInfo.socket->getOutputStream()->write(tp.packetBody, tp.packetLen);
      smsc_log_info(_logger, "send message=[%s] to link=[%s], linkId=%d", utilx::hexdmp(tp.packetBody, tp.packetLen).c_str(), linkInfo.toString().c_str(), connect_num);
      return CallResult(OK, connect_num);
    } else {
      smsc_log_error(_logger, "SccpUser::unitdata_req::: connection for link=[%s] hasn't been binded, linkId=%d", linkInfo.toString().c_str(), connect_num);
      return CallResult(NOT_BINDED, connect_num);
    }
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SccpUser::unitdata_req::: caught SystemError exception=[%s]", ex.what());
    return CallResult(SYSTEM_ERROR, connect_num);
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SccpUser::unitdata_req::: caught unexpected exception=[%s]", ex.what());
    return CallResult(SYSTEM_MALFUNCTION, connect_num);
  }
}

SccpApi::ErrorCode_e
SccpUser::msgRecv(MessageInfo* msg_info, uint32_t timeout)
{
  if ( !_wasInitialized )
    return LIB_NOT_INITIALIZED;

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
        iStream=availableData->owner;

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

        msg_info->msgData.reset(static_cast<unsigned>(cacheEntry->expectedMessageSize +
            sizeof(cacheEntry->expectedMessageSize)));
        msg_info->messageType = cacheEntry->ringBuf.readUint32();
        switch(msg_info->messageType) {
          case SccpMessageId::BIND_MSGCODE:
          case SccpMessageId::BIND_CONFIRM_MSGCODE:
          case SccpMessageId::UNBIND_MSGCODE:
          case SccpMessageId::N_UNITDATA_REQ_MSGCODE:
          case SccpMessageId::N_UNITDATA_IND_MSGCODE:
          case SccpMessageId::N_NOTICE_IND_MSGCODE:
          case SccpMessageId::N_PCSTATE_IND_MSGCODE:
          case SccpMessageId::N_STATE_IND_MSGCODE:
          case SccpMessageId::N_COORD_IND_MSGCODE:
            break;
          default:
            smsc_log_error(_logger, "SccpUser::msgRecv::: got message with unknown messageType value=%u",
                           msg_info->messageType);
            return GOT_UNKOWN_MESSAGE;
        }
        uint32_t msgTypeField = htonl(msg_info->messageType);

        msg_info->msgData.Append(reinterpret_cast<uint8_t*>(&lenField), static_cast<unsigned>(sizeof(lenField)));
        msg_info->msgData.Append(reinterpret_cast<uint8_t*>(&msgTypeField), static_cast<unsigned>(sizeof(msgTypeField)));

        cacheEntry->ringBuf.readArray(msg_info->msgData.getCurPtr(), cacheEntry->expectedMessageSize - sizeof(msg_info->messageType));
        msg_info->msgData.setPos(static_cast<unsigned>(msg_info->msgData.getPos() +
            cacheEntry->expectedMessageSize - sizeof(msg_info->messageType)));
        msg_info->connectNum = static_cast<LinkInputStream*>(iStream)->getConnectNum();

        //delete cacheEntry;
        //_packetsCache.erase(iter);
        smsc_log_debug(_logger,"msgRecv: after cacheEntry.expectedMessageSize=%d, cacheEntry.getSizeOfAvailData()=%d",cacheEntry->expectedMessageSize,cacheEntry->ringBuf.getSizeOfAvailData());
        cacheEntry->expectedMessageSize=0;
        if(cacheEntry->ringBuf.getSizeOfAvailData()>=4) {
          cacheEntry->expectedMessageSize=cacheEntry->ringBuf.readUint32();
          if(cacheEntry->expectedMessageSize<=cacheEntry->ringBuf.getSizeOfAvailData())
            availableData=cacheEntry;
          else
            availableData=0;
        } else
          availableData=0;

        return OK;
      }
    }

    return OK;
  } catch (smsc::util::SystemError& ex) {
    smsc_log_error(_logger, "SccpUser::msgRecv::: caught SystemError exception=[%s]", ex.what());
    return SYSTEM_ERROR;
  } catch (smsc::util::Exception& ex) {
    smsc_log_error(_logger, "SccpUser::msgRecv::: caught unexpected exception=[%s]", ex.what());
    return SYSTEM_MALFUNCTION;
  }
}

unsigned
SccpUser::getConnectsCount() const
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);
  return static_cast<unsigned>(_knownLinks.size());
}

unsigned
SccpUser::getConnNumByPolicy()
{
  if ( _trafficMode == OVERRIDE) {
    return _lastUsedConnIdx;
  } else if ( _trafficMode = LOADSHARE ) {
    smsc::core::synchronization::MutexGuard synchronize(_lastUsedConnIdxLock);
    _lastUsedConnIdx = static_cast<unsigned>((_lastUsedConnIdx + 1) % _knownLinks.size());
    return _lastUsedConnIdx;
  } else
    throw SccpLibException("SccpUser::getConnNumByPolicy::: invalid traffic mode=[%d]", _trafficMode);
}

SccpUser::LinkInfo::LinkInfo()
  : port(0),
    socket(NULL), connectionState(linkNOT_CONNECTED), inputStream(NULL) {}

SccpUser::LinkInfo::LinkInfo(const std::string& link_name, const std::string& a_host, in_port_t port)
  : linkName(link_name), host(a_host), port(port),
    socket(NULL), connectionState(linkNOT_CONNECTED), inputStream(NULL)
{
  if ( link_name == "" ) throw SccpLibException("LinkInfo::LinkInfo::: empty link name value");
  if ( host == "" ) throw SccpLibException("LinkInfo::LinkInfo::: empty host value");
}

std::string
SccpUser::LinkInfo::toString() const
{
  char strBuf[256];
  snprintf(strBuf, sizeof(strBuf), "link=[%s],host=[%s],port=[%d]",linkName.c_str(), host.c_str(), port);
  return std::string(strBuf);
}

SccpUser::LinkInputStream::LinkInputStream(corex::io::InputStream* i_stream, unsigned int connect_num)
  : _iStream(i_stream), _connectNum(connect_num) {}

ssize_t
SccpUser::LinkInputStream::read(uint8_t *buf, size_t buf_sz)
{
  return _iStream->read(buf, buf_sz);
}

ssize_t
SccpUser::LinkInputStream::readv(const struct iovec *iov, int iovcnt)
{
  return _iStream->readv(iov, iovcnt);
}

corex::io::IOObject*
SccpUser::LinkInputStream::getOwner()
{
  return _iStream->getOwner();
}

unsigned int
SccpUser::LinkInputStream::getConnectNum() const
{
  return _connectNum;
}

SccpUser::traffic_mode_t
SccpUser::convertStringToTrafficModeValue(const std::string& traffic_mode, const std::string& where)
{
  if ( traffic_mode == "loadshare" )
    return LOADSHARE;
  else if ( traffic_mode == "override" )
    return OVERRIDE;
  else {
    const std::string fmtStr = where + "convertStringToTrafficModeValue::: wrong traffic-mode parameter value=[%s]";
    throw smsc::util::Exception(fmtStr.c_str(), traffic_mode.c_str());
  }
}

}}}
