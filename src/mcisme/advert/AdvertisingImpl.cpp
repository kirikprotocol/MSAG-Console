#include "AdvertisingImpl.h"
#include "AdvertErrors.h"
#include <string.h>
#include <util/Exception.hpp>
#include <util/debug.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include <mcisme/Exceptions.hpp>

#include <sstream>
#include <iomanip>
namespace smsc {
namespace mcisme {

std::string
hexdmp(const uchar_t* buf, size_t bufSz)
{
  std::ostringstream hexBuf;
  hexBuf.fill('0');
  hexBuf << std::hex;
  for (size_t i=0; i<bufSz; ++i)
    hexBuf << std::setw(2) << (uint32_t) buf[i];

  return hexBuf.str();
}

void
AdvertisingImpl::init(int connectTimeout)
{
  if ( _socket.Init(_host.c_str(), _port, _timeout) )
    throw util::Exception("AdvertisingImpl::init::: can't init socket to connect to %s - '%s'", toString().c_str(), strerror(errno));

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

void
AdvertisingImpl::readFromSocket(char *dataBuf, int bytesToRead, const std::string& where)
{
  int rd = 0, res;
  smsc_log_debug(_logger, "AdvertisingImpl::readFromSocket: _timeout=%d, bytesToRead=%d", _timeout, bytesToRead);
  while (rd<bytesToRead)
  {
    if ( _timeout > 0 ) {
      res = _socket.canRead(_timeout);
      if ( res < 0 ) {
        _isConnected = false;
        throw NetworkException("AdvertisingImpl::readFromSocket::: read socket error = %d", errno);
      } else if ( !res )
        throw TimeoutException("AdvertisingImpl::readFromSocket::: read timeout was expired (timeout value=%d)", _timeout);
    }
    res = _socket.Read(dataBuf+rd,bytesToRead-rd);
    if (res<0) {
      _isConnected = false;
      throw NetworkException("AdvertisingImpl::readFromSocket::: read socket error = %d", errno);
    } else if (!res) {
      _isConnected = false;
      throw NetworkException("AdvertisingImpl::readFromSocket::: connection closed by remote side");
    }
    smsc_log_debug(_logger, "AdvertisingImpl::readFromSocket: _socket.read returned %d",res);
    rd+=res;
  }
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
AdvertisingImpl::getBanner(const std::string& abonent,
                           const std::string& serviceName,
                           uint32_t transportType, uint32_t charSet,
                           std::string* banner,
                           BannerResponseTrace* bannerRespTrace)
{
  if ( !_isConnected ) return ERR_ADV_NOT_CONNECTED;

  BannerRequest req(abonent, serviceName, transportType, charSet);
  uint32_t rc = getBanner(req, bannerRespTrace);
  *banner = req.banner;
  return  rc;
}

void
AdvertisingImpl::rollbackBanner(uint32_t transactionId,
                                uint32_t bannerId,
                                uint32_t ownerId,
                                uint32_t rotatorId)
{
  BannerRequest banReqInfo(transactionId, bannerId, ownerId, rotatorId);
  sendErrorInfo(banReqInfo, ERR_ADV_OTHER, "AdvertisingImpl::rollbackBanner:::");
}

uint32_t
AdvertisingImpl::getBanner(BannerRequest& banReq, BannerResponseTrace* bannerRespTrace)
{
  advertising_item curAdvItem(&banReq);

  util::SerializationBuffer req;
  // заполнение буфера протокольной команды
  uint32_t len = prepareBannerReqCmd(&req, &banReq);

  int rc = sendRequestAndGetResponse(&curAdvItem, &req, len, bannerRespTrace);
  if (rc != 0)
  {
    writeErrorToLog((char*)"AdvertisingImpl::getBanner", rc);
    sendErrorInfo(banReq, rc, "AdvertisingImpl::getBanner:::");
  }

  return rc;
}

int
AdvertisingImpl::sendRequestAndGetResponse(advertising_item* advItem, util::SerializationBuffer* req, uint32_t req_len, BannerResponseTrace* bannerRespTrace)
{
  writeToSocket(req->getBuffer(), req_len, "AdvertisingImpl::sendRequestAndGetResponse");

  return readAdvert(advItem, bannerRespTrace);
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
