#include <mcisme/Exceptions.hpp>
#include "BEProtocolV2SimpleClient.hpp"
#include "AdvertErrors.h"

namespace smsc {
namespace mcisme {

std::string
hexdmp(const uchar_t* buf, size_t bufSz);

BEProtocolV2SimpleClient::BEProtocolV2SimpleClient(const std::string& host, int port)
  : AdvertisingImpl(host, port), _pos(0), _totalPacketLen(0)
{}

uint32_t
BEProtocolV2SimpleClient::readPacket()
{
  try
  {
    int bytes_to_read;
    if (!_totalPacketLen)
      bytes_to_read = CMD_HEADER_SIZE - _pos;
    else
      bytes_to_read = _totalPacketLen - _pos;

    int read_sz = readFromSocket(_buf + _pos, bytes_to_read, "BEProtocolV2SimpleClient::readPacket");
    _pos += read_sz;
    if (!_totalPacketLen)
    {
      if (_pos < CMD_HEADER_SIZE)
        return 0;

      uint32_t *s = (uint32_t*)_buf;
      _totalPacketLen = ntohl(s[1]) + CMD_HEADER_SIZE;

      if (_totalPacketLen > sizeof(_buf))
      {
        smsc_log_warn(_logger, "BEProtocolV2SimpleClient::readPacket::: got too long packet, totalPacketLen==%d", _totalPacketLen);
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
BEProtocolV2SimpleClient::parseBannerResponse(const char* packet, uint32_t totalPacketLen,
                                              uint32_t* gotTransactId, std::string* banner,
                                              uint32_t* bannerId, uint32_t* ownerId,
                                              uint32_t* rotatorId)
{
  uint32_t *s = (uint32_t*)packet;
  uint32_t cmdType = ntohl(s[0]);

  if ( cmdType != CMD_BANNER_RSP ) {
    smsc_log_error(_logger, "BEProtocolV2SimpleClient::parseBannerResponse::: incorrect packet type %d", cmdType);
    throw ProtocolError();
  }

  uint32_t offset = CMD_HEADER_SIZE;

  while (offset < totalPacketLen) {
    const char* curPtrInBuf = packet + offset;
    uint32_t tag, len;
    offset += extractParamHeader(curPtrInBuf, totalPacketLen - offset, &tag, &len);
    curPtrInBuf = packet + offset;
    switch (tag) {
    case TRANSACTION_ID_TAG:
      offset += extractParamValue(curPtrInBuf, totalPacketLen - offset, gotTransactId);
      break;
    case BANNER_ID_TAG:
      offset += extractParamValue(curPtrInBuf, totalPacketLen - offset, bannerId);
      break;
    case OWNER_ID_TAG:
      offset += extractParamValue(curPtrInBuf, totalPacketLen - offset, ownerId);
      break;
    case ROTATOR_ID_TAG:
      offset += extractParamValue(curPtrInBuf, totalPacketLen - offset, rotatorId);
      break;
    case BANNER_BODY_TAG:
      offset += extractParamValue(curPtrInBuf, totalPacketLen - offset, banner, len);
      break;
    case CLICK_PRICE_TAG:
      {
        uint32_t clickPrice;
        offset += extractParamValue(curPtrInBuf, totalPacketLen - offset, &clickPrice);
      }
      break;
    case CHARSET_TAG:
      {
        uint32_t charset;
        offset += extractParamValue(curPtrInBuf, totalPacketLen - offset, &charset);
      }
      break;
    default:
      smsc_log_error(_logger, "BEProtocolV2SimpleClient::parseBannerResponse::: got unknown parameter type %d", tag);
      throw ProtocolError();
    }
  }
}

uint32_t
BEProtocolV2SimpleClient::extractParamValue(const char* curPtrInBuf, uint32_t restSizeOfBuff,
                                            uint32_t* value)
{
  if ( restSizeOfBuff < sizeof(uint32_t) ) {
    smsc_log_error(_logger, "BEProtocolV2SimpleClient::extractParamValue(uint32_r)::: buffer to small");
    throw ProtocolError();
  }

  memcpy(reinterpret_cast<uint8_t*>(value), curPtrInBuf, sizeof(uint32_t));

  *value = ntohl(*value);

  return static_cast<uint32_t>(sizeof(uint32_t));
}

uint32_t
BEProtocolV2SimpleClient::extractParamValue(const char* curPtrInBuf, uint32_t restSizeOfBuff,
                                            std::string* value, uint32_t paramLen)
{
  if ( restSizeOfBuff < paramLen ) {
    smsc_log_error(_logger, "BEProtocolV2SimpleClient::extractParamValue(uint32_r)::: buffer to small");
    throw ProtocolError();
  }

  value->assign(curPtrInBuf, paramLen);

  return paramLen;
}

uint32_t
BEProtocolV2SimpleClient::extractParamHeader(const char* curPtrInBuf,
                                             uint32_t restSizeOfBuff,
                                             uint32_t* tag,
                                             uint32_t* len)
{
  if ( restSizeOfBuff < PARAM_HDR_SZ ) {
    smsc_log_error(_logger, "BEProtocolV2SimpleClient::extractTag::: buffer to small");
    throw ProtocolError();
  }

  memcpy(reinterpret_cast<uint8_t*>(tag), curPtrInBuf, sizeof(uint32_t));
  *tag = ntohl(*tag);

  memcpy(reinterpret_cast<uint8_t*>(len), curPtrInBuf + sizeof(uint32_t), sizeof(uint32_t));
  *len = ntohl(*len);

  return PARAM_HDR_SZ;
}

banner_read_stat
BEProtocolV2SimpleClient::readAdvert(std::string* banner,
                                     BannerResponseTrace* banner_resp_trace)
{
  smsc_log_debug(_logger, "BEProtocolV2SimpleClient::readAdvert");

  static BannerResponseTrace emptyBannerResponseTrace;
  *banner_resp_trace = emptyBannerResponseTrace;
  try {
    uint32_t totalPacketLen = readPacket();
    if (!totalPacketLen)
      return CONTINUE_READ_PACKET;

    smsc_log_debug(_logger, "BEProtocolV2SimpleClient::readAdvert::: got packet from BE: [%s]", hexdmp((uchar_t*)_buf, totalPacketLen).c_str());
    parseBannerResponse(_buf, totalPacketLen, &banner_resp_trace->transactionId,
                        banner, &banner_resp_trace->bannerId, &banner_resp_trace->ownerId,
                        &banner_resp_trace->rotatorId);

    smsc_log_info(_logger, "got BannerResponse: transactionId=%u, bannerId=%u, ownerId=%u, rotatorId=%u",
                  banner_resp_trace->transactionId, banner_resp_trace->bannerId, banner_resp_trace->ownerId, banner_resp_trace->rotatorId);
  } catch (TimeoutException& ex) {
    smsc_log_error(_logger, "BEProtocolV2SimpleClient::readAdvert::: caught TimeoutException [%s]", ex.what());
    return ERR_ADV_TIMEOUT;
  } catch (ProtocolError& ex) {
    smsc_log_error(_logger, "BEProtocolV2SimpleClient::readAdvert::: caught ProtocolError exception");
    return ERR_ADV_OTHER;
  }

  return BANNER_OK;
}

uint32_t
BEProtocolV2SimpleClient::prepareBannerReqCmd(util::SerializationBuffer* req /* �����*/,
                                              BannerRequest* par /*��������� ������� ������*/)
{
  uint32_t totalPacketSize = prepareHeader(CMD_BANNER_REQ, static_cast<uint32_t>(GET_BANNER_REQ_BODY_LEN + par->abonent.length() + par->serviceName.length()), req);

  //TransactId
  req->WriteNetInt32(TRANSACTION_ID_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t))); //Param length
  req->WriteNetInt32(par->getId());   //Param body

  // banner id
  req->WriteNetInt32(BANNER_ID_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(par->bannerId);

  // owner id
  req->WriteNetInt32(OWNER_ID_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(par->ownerId);

  // rotator id
  req->WriteNetInt32(ROTATOR_ID_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(par->rotatorId);

  // abonent
  req->WriteNetInt32(ABONENT_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(par->abonent.length()));
  req->Write(par->abonent.c_str(), par->abonent.length());

  switch (par->transportType)
  {
  case TRANSPORT_TYPE_SMS:
  case TRANSPORT_TYPE_USSD:
  case TRANSPORT_TYPE_HTTP:
  case TRANSPORT_TYPE_MMS:
    break;
  default:
    throw util::Exception("BEProtocolV2SimpleClient::prepareBannerReqCmd::: invalid transportType=%d", par->transportType);
  }

  // banner max length
  req->WriteNetInt32(BANNER_MAX_LEN_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(par->maxBannerSize)));
  req->WriteNetInt32(par->maxBannerSize);

  //charSet
  req->WriteNetInt32(CHARSET_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(par->charSet)));
  req->WriteNetInt32(par->charSet);

  // service_Name
  req->WriteNetInt32(SERVICE_NAME_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(par->serviceName.length()));
  req->Write(par->serviceName.c_str(), par->serviceName.length());

  // transportType
  req->WriteNetInt32(TRANSPORT_TYPE_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(par->transportType)));
  req->WriteNetInt32(par->transportType);

  return totalPacketSize;
}

uint32_t
BEProtocolV2SimpleClient::prepareErrorInfoCmd(util::SerializationBuffer* req /* �����*/,
                                              const BannerRequest& par /*��������� ������� ������*/,
                                              uint32_t errCode)
{
  uint32_t totalPacketSize = prepareHeader(CMD_ROLLBACK_REQ, CMD_ROLLBACK_REQ_BODY_LEN, req);

  //TransactId
  req->WriteNetInt32(TRANSACTION_ID_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t))); //Param length
  req->WriteNetInt32(par.getId());   //Param body

  // banner id
  req->WriteNetInt32(BANNER_ID_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(par.bannerId);

  // owner id
  req->WriteNetInt32(OWNER_ID_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(par.ownerId);

  // rotator id
  req->WriteNetInt32(ROTATOR_ID_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(par.rotatorId);

  // error code
  req->WriteNetInt32(ERROR_CODE_TAG);
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(errCode);

  return totalPacketSize;
}

void
BEProtocolV2SimpleClient::sendErrorInfo(const BannerRequest& banReq,
                                        int rc)
{
  util::SerializationBuffer req;
  int len;
  if (rc == ERR_ADV_TIMEOUT)
    len = prepareErrorInfoCmd(&req, banReq, BANNER_RESPONSE_TIMEOUT);
  else
    len = prepareErrorInfoCmd(&req, banReq, BANNER_OTHER_ERROR);

  writeToSocket(req.getBuffer(), len, "BEProtocolV2SimpleClient::sendErrorInfo");
}

}}
