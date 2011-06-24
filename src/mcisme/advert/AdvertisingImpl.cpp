#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "AdvertisingImpl.h"
#include "AdvertErrors.h"
#include "util/Exception.hpp"
#include "util/debug.h"

#include "mcisme/Exceptions.hpp"
#include "SendBannerIdRegistry.hpp"

namespace smsc {
namespace mcisme {

extern std::string
hexdmp(const uchar_t* buf, size_t bufSz);

void
AdvertisingImpl::init(int connectTimeout)
{
  smsc_log_debug(_logger, "AdvertisingImpl::init::: try connect to BannerEngine (host%s,port=%d)", _host.c_str(), _port);
  if ( _socket.Init(_host.c_str(), _port, 0) )
    throw util::Exception("AdvertisingImpl::init::: can't init socket to connect to %s - '%s'", toString().c_str(), strerror(errno));

  smsc_log_info(_logger, "AdvertisingImpl::init::: connect to BannerEngine (host%s,port=%d) has been established", _host.c_str(), _port);
  _socket.setConnectTimeout(connectTimeout);

  if ( _socket.Connect() )
    throw util::Exception("AdvertisingImpl::init::: can't establish connection to %s - '%s'", toString().c_str(), strerror(errno));

  _isConnected = true;
}

bool
AdvertisingImpl::reinit(int connectTimeout)
{
  _socket.setConnectTimeout(connectTimeout);

  if ( _socket.Connect() ) {
    smsc_log_error(_logger, "AdvertisingImpl::reinit::: can't establish connection to %s - %s", toString().c_str(), strerror(errno));
    return false;
  }

  _isConnected = true;
  return true;
}

void
AdvertisingImpl::writeErrorToLog(char* where, int errCode)
{
  smsc_log_error(_logger, "%s, error %d", where, errCode);
}

void
AdvertisingImpl::writeToSocket(const void* dataBuf, int dataSz, const std::string& where)
{
  smsc_log_debug(_logger,"AdvertisingImpl::writeToSocket::: try write data=[%s]", hexdmp((uint8_t*)dataBuf, dataSz).c_str());
  if ( _socket.WriteAll((char *)dataBuf, dataSz) < 0 ) {
    _isConnected = false;
    std::string errMsg = where + " socket write error [" + strerror(errno) + "]";
    smsc_log_warn(_logger, errMsg);
    throw NetworkException(errMsg.c_str());
  }
}

int
AdvertisingImpl::readFromSocket(char *dataBuf, int bytesToRead, const std::string& where)
{
  int res;
  smsc_log_debug(_logger, "AdvertisingImpl::readFromSocket: bytesToRead=%d", bytesToRead);
  res = _socket.Read(dataBuf,bytesToRead);
  if (res<0) {
    _isConnected = false;
    throw NetworkException("AdvertisingImpl::readFromSocket::: read socket error = %d", errno);
  } else if (!res) {
    _isConnected = false;
    throw NetworkException("AdvertisingImpl::readFromSocket::: connection closed by remote side");
  }
  smsc_log_debug(_logger, "AdvertisingImpl::readFromSocket: dump of data gotten from socket %s",
                 hexdmp((uchar_t*)dataBuf, res).c_str());
  return res;
}

uint32_t
AdvertisingImpl::prepareHeader(uint32_t cmndType, uint32_t reqBodyLen, util::SerializationBuffer* req)
{
  uint32_t totalPacketSize = static_cast<uint32_t>(reqBodyLen + CMD_HEADER_SIZE);
  req->resize(totalPacketSize);
  req->setPos(0);

  req->WriteNetInt32(cmndType); //Command Type
  req->WriteNetInt32(reqBodyLen);  //Command length

  return totalPacketSize;
}

uint32_t
AdvertisingImpl::sendBannerRequest(const std::string& abonent,
                                   const std::string& service_name,
                                   uint32_t transport_type, uint32_t char_set,
                                   uint32_t max_banner_size, MCEventOut* mc_event_out)
{
  if ( !_isConnected ) return ERR_ADV_NOT_CONNECTED;

  BannerRequest* req = new BannerRequest(abonent, service_name, transport_type, char_set, max_banner_size, mc_event_out);
  sendBannerRequest(req);

  return BANNER_OK;
}

void
AdvertisingImpl::rollbackBanner(uint32_t transactionId,
                                uint32_t bannerId,
                                uint32_t ownerId,
                                uint32_t rotatorId)
{
  BannerRequest banReqInfo(transactionId, bannerId, ownerId, rotatorId);
  sendErrorInfo(banReqInfo, ERR_ADV_OTHER);
}

void
AdvertisingImpl::sendBannerRequest(BannerRequest* banReq)
{
  util::SerializationBuffer req;
  // ���������� ������ ������������ �������
  uint32_t len = prepareBannerReqCmd(&req, banReq);

  writeToSocket(req.getBuffer(), len, "AdvertisingImpl::sendBannerRequest");
  banReq->fd = _socket.getSocket();
  SendBannerIdRegistry::getInstance().saveSentBannerInfo(banReq);
}

void
AdvertisingImpl::generateUnrecoveredProtocolError()
{
  _socket.Close();
  throw UnrecoveredProtocolError();
}

std::string
AdvertisingImpl::toString() const
{
  char strBuf[128];
  snprintf(strBuf, sizeof(strBuf), "%s.%d", _host.c_str(), _port);

  return strBuf;
}

} // advert
} // scag
