#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "AdvertisingImpl.h"
#include "AdvertErrors.h"
#include "util/Exception.hpp"
#include "util/debug.h"

#include "SendBannerIdRegistry.hpp"

namespace smsc {
namespace mcisme {

extern std::string
hexdmp(const uchar_t* buf, size_t bufSz);

void
AdvertisingImpl::init(int connect_timeout)
{
  smsc_log_debug(_logger, "AdvertisingImpl::init::: try connect to BannerEngine (host%s,port=%d)", _host.c_str(), _port);
  if ( _socket.Init(_host.c_str(), _port, 0) )
    throw util::Exception("AdvertisingImpl::init::: can't init socket to connect to %s - '%s'", toString().c_str(), strerror(errno));

  smsc_log_info(_logger, "AdvertisingImpl::init::: connect to BannerEngine (host%s,port=%d) has been established", _host.c_str(), _port);
  _socket.setConnectTimeout(connect_timeout);

  if ( _socket.Connect() )
    throw util::Exception("AdvertisingImpl::init::: can't establish connection to %s - '%s'", toString().c_str(), strerror(errno));

  _isConnected = true;
}

bool
AdvertisingImpl::reinit(int connect_timeout)
{
  _socket.setConnectTimeout(connect_timeout);

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
AdvertisingImpl::writeToSocket(const void* data_buf, int data_sz, const std::string& where)
{
  smsc_log_debug(_logger,"AdvertisingImpl::writeToSocket::: try write data=[%s]", hexdmp((uint8_t*)data_buf, data_sz).c_str());
  if ( _socket.WriteAll((char *)data_buf, data_sz) < 0 ) {
    _isConnected = false;
    std::string errMsg = where + " socket write error [" + strerror(errno) + "]";
    smsc_log_warn(_logger, errMsg);
    throw NetworkException(errMsg.c_str());
  }
}

int
AdvertisingImpl::readFromSocket(char *data_buf, int bytes_to_read, const std::string& where)
{
  int res;
  smsc_log_debug(_logger, "AdvertisingImpl::readFromSocket: bytesToRead=%d", bytes_to_read);
  res = _socket.Read(data_buf,bytes_to_read);
  if (res<0) {
    _isConnected = false;
    throw NetworkException("AdvertisingImpl::readFromSocket::: read socket error = %d", errno);
  } else if (!res) {
    _isConnected = false;
    throw NetworkException("AdvertisingImpl::readFromSocket::: connection closed by remote side");
  }
  smsc_log_debug(_logger, "AdvertisingImpl::readFromSocket: dump of data gotten from socket %s",
                 hexdmp((uchar_t*)data_buf, res).c_str());
  return res;
}

uint32_t
AdvertisingImpl::prepareHeader(uint32_t cmd_type, uint32_t req_body_len, util::SerializationBuffer* req)
{
  uint32_t totalPacketSize = static_cast<uint32_t>(req_body_len + CMD_HEADER_SIZE);
  req->resize(totalPacketSize);
  req->setPos(0);

  req->WriteNetInt32(cmd_type); //Command Type
  req->WriteNetInt32(req_body_len);  //Command length

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
AdvertisingImpl::rollbackBanner(uint32_t transaction_id,
                                uint32_t banner_id,
                                uint32_t owner_id,
                                uint32_t rotator_id,
                                const std::string& service_name)
{
  BannerRequest banReqInfo(transaction_id, banner_id, owner_id, rotator_id, service_name);
  sendErrorInfo(banReqInfo, ERR_ADV_OTHER);
}

void
AdvertisingImpl::sendBannerRequest(BannerRequest* ban_req)
{
  util::SerializationBuffer req;
  // ���������� ������ ������������ �������
  uint32_t len = prepareBannerReqCmd(&req, ban_req);

  writeToSocket(req.getBuffer(), len, "AdvertisingImpl::sendBannerRequest");
  ban_req->fd = _socket.getSocket();
  SendBannerIdRegistry::getInstance().saveSentBannerInfo(ban_req);
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

uint32_t
AdvertisingImpl::readPacket()
{
  try
  {
    int bytes_to_read;
    if (!_totalPacketLen)
      bytes_to_read = CMD_HEADER_SIZE - _pos;
    else
      bytes_to_read = _totalPacketLen - _pos;

    int read_sz = readFromSocket(_buf + _pos, bytes_to_read, "AdvertisingImpl::readPacket");
    _pos += read_sz;
    if (!_totalPacketLen)
    {
      if (_pos < CMD_HEADER_SIZE)
        return 0;

      uint32_t *s = (uint32_t*)_buf;
      _totalPacketLen = ntohl(s[1]) + CMD_HEADER_SIZE;

      if (_totalPacketLen > sizeof(_buf))
      {
        smsc_log_warn(_logger, "AdvertisingImpl::readPacket::: got too long packet, totalPacketLen==%d", _totalPacketLen);
        _totalPacketLen = 0; _pos = 0;
        generateUnrecoveredProtocolError();
      }

      return 0;
    }

    if (_pos == _totalPacketLen)
    {
      // we have read total packet
      uint32_t totalPacketLen = _totalPacketLen;
      _pos=0; _totalPacketLen = 0;
      return totalPacketLen;
    }

    return 0;
  } catch (NetworkException& ex) {
    _pos=0; _totalPacketLen = 0;
    throw;
  }
}

void
AdvertisingImpl::sendErrorInfo(const BannerRequest& ban_req,
                               int rc)
{
  util::SerializationBuffer req;
  int len;
  if (rc == ERR_ADV_TIMEOUT)
    len = prepareErrorInfoCmd(&req, ban_req, BANNER_RESPONSE_TIMEOUT);
  else
    len = prepareErrorInfoCmd(&req, ban_req, BANNER_OTHER_ERROR);

  writeToSocket(req.getBuffer(), len, "AdvertisingImpl::sendErrorInfo");
}

uint32_t
AdvertisingImpl::extractParamValue(const char* cur_ptr_in_buf, uint32_t rest_size_of_buff,
                                   uint32_t* value)
{
  if ( rest_size_of_buff < sizeof(uint32_t) ) {
    smsc_log_error(_logger, "AdvertisingImpl::extractParamValue(uint32_r)::: buffer to small");
    throw ProtocolError();
  }

  memcpy(reinterpret_cast<uint8_t*>(value), cur_ptr_in_buf, sizeof(uint32_t));

  *value = ntohl(*value);

  return static_cast<uint32_t>(sizeof(uint32_t));
}

uint32_t
AdvertisingImpl::extractParamValue(const char* cur_ptr_in_buf, uint32_t rest_size_of_buff,
                                   std::string* value, uint32_t param_len)
{
  if ( rest_size_of_buff < param_len ) {
    smsc_log_error(_logger, "AdvertisingImpl::extractParamValue(uint32_r)::: buffer to small");
    throw ProtocolError();
  }

  value->assign(cur_ptr_in_buf, param_len);

  return param_len;
}

uint32_t
AdvertisingImpl::extractParamHeader(const char* cur_ptr_in_buf,
                                    uint32_t rest_size_of_buff,
                                    uint32_t* tag,
                                    uint32_t* len)
{
  if ( rest_size_of_buff < PARAM_HDR_SZ ) {
    smsc_log_error(_logger, "AdvertisingImpl::extractTag::: buffer to small");
    throw ProtocolError();
  }

  memcpy(reinterpret_cast<uint8_t*>(tag), cur_ptr_in_buf, sizeof(uint32_t));
  *tag = ntohl(*tag);

  memcpy(reinterpret_cast<uint8_t*>(len), cur_ptr_in_buf + sizeof(uint32_t), sizeof(uint32_t));
  *len = ntohl(*len);

  return PARAM_HDR_SZ;
}

banner_read_stat
AdvertisingImpl::readAdvert(std::string* banner,
                             BannerResponseTrace* banner_resp_trace)
{
  smsc_log_debug(_logger, "AdvertisingImpl::readAdvert");

  static BannerResponseTrace emptyBannerResponseTrace;
  *banner_resp_trace = emptyBannerResponseTrace;
  try {
    uint32_t totalPacketLen = readPacket();
    if (!totalPacketLen)
      return CONTINUE_READ_PACKET;

    smsc_log_debug(_logger, "AdvertisingImpl::readAdvert::: got packet from BE: [%s]", hexdmp((uchar_t*)_buf, totalPacketLen).c_str());
    parseBannerResponse(_buf, totalPacketLen, &banner_resp_trace->transactionId,
                        banner, &banner_resp_trace->bannerId, &banner_resp_trace->ownerId,
                        &banner_resp_trace->rotatorId, &banner_resp_trace->serviceName);
  } catch (ProtocolError& ex) {
    smsc_log_error(_logger, "AdvertisingImpl::readAdvert::: caught ProtocolError exception");
    return ERR_ADV_OTHER;
  }

  return BANNER_OK;
}

} // advert
} // scag
