#include <mcisme/Exceptions.hpp>
#include "BEProtocolV2SimpleClient.hpp"
#include "AdvertErrors.h"

namespace smsc {
namespace mcisme {

std::string
hexdmp(const uchar_t* buf, size_t bufSz);

BEProtocolV2SimpleClient::BEProtocolV2SimpleClient(const std::string& host, int port, int timeout)
  : AdvertisingImpl(host, port, timeout)
{}

uint32_t
BEProtocolV2SimpleClient::readPacket(char* buf, size_t bufSize)
{
  uint32_t *s = (uint32_t*)buf;

  readFromSocket(buf, CMD_HEADER_SIZE, "BEProtocolV2SimpleClient::readPacket");

  buf += CMD_HEADER_SIZE;

  uint32_t packetLen = ntohl(s[1]);

  if (packetLen > bufSize - CMD_HEADER_SIZE)
  {
    smsc_log_warn(_logger, "BEProtocolV2SimpleClient::readPacket::: got too long packet, packetLen==%d", packetLen);
    generateUnrecoveredProtocolError();
  }

  readFromSocket(buf, packetLen, "BEProtocolV2SimpleClient::readPacket");

  return packetLen + CMD_HEADER_SIZE;
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

uint32_t
BEProtocolV2SimpleClient::readAdvert(advertising_item* advItem,
                                     BannerResponseTrace* bannerRespTrace)
{
  smsc_log_debug(_logger, "BEProtocolV2SimpleClient::readAdvert");

  BannerResponseTrace emptyBannerResponseTrace;
  try {
    while (true) {
      char incomingPacketBuf[MAX_PACKET_LEN];

      uint32_t totalPacketLen = readPacket(incomingPacketBuf, sizeof(incomingPacketBuf));

      smsc_log_debug(_logger, "BEProtocolV2SimpleClient::readAdvert::: got packet from BE: [%s]", hexdmp((uint8_t*)incomingPacketBuf, totalPacketLen).c_str());
      BannerResponseTrace gotBannerRespTrace;
      std::string gotBannerString;
      parseBannerResponse(incomingPacketBuf, totalPacketLen, &gotBannerRespTrace.transactionId,
                          &gotBannerString, &gotBannerRespTrace.bannerId, &gotBannerRespTrace.ownerId,
                          &gotBannerRespTrace.rotatorId);

      smsc_log_info(_logger, "got BannerResponse: transactionId=%d, bannerId=%d, ownerId=%d, rotatorId=%d", gotBannerRespTrace.transactionId, gotBannerRespTrace.bannerId, gotBannerRespTrace.ownerId, gotBannerRespTrace.rotatorId);
      if ( gotBannerRespTrace.transactionId == advItem->TransactID ) {
        *bannerRespTrace = gotBannerRespTrace;
        advItem->banReq->banner = gotBannerString;
        break;
      } else {
        smsc_log_error(_logger, "BEProtocolV2SimpleClient::readAdvert::: wrong transactionId value=[%d], expected value=[%d], send rollback message for this response", gotBannerRespTrace.transactionId, advItem->TransactID);

        sendErrorInfo(BannerRequest(gotBannerRespTrace.transactionId, gotBannerRespTrace.bannerId, gotBannerRespTrace.ownerId, gotBannerRespTrace.rotatorId), ERR_ADV_TIMEOUT, "BEProtocolV2SimpleClient::readAdvert");
      }
    }
  } catch (TimeoutException& ex) {
    smsc_log_error(_logger, "BEProtocolV2SimpleClient::readAdvert::: catched TimeoutException [%s]", ex.what());
    *bannerRespTrace = emptyBannerResponseTrace;
  } catch (ProtocolError& ex) {
    smsc_log_error(_logger, "BEProtocolV2SimpleClient::readAdvert::: catched ProtocolError exception");
    *bannerRespTrace = emptyBannerResponseTrace;
  }

  return 0;
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
                                        int rc,
                                        const std::string& where)
{
  util::SerializationBuffer req;
  int len;
  if (rc == ERR_ADV_TIMEOUT)
    len = prepareErrorInfoCmd(&req, banReq, BANNER_RESPONSE_TIMEOUT);
  else
    len = prepareErrorInfoCmd(&req, banReq, BANNER_OTHER_ERROR);

  writeToSocket(req.getBuffer(), len, where);
}

}}
