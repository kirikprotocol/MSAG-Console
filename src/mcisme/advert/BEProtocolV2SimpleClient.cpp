#include "mcisme/Exceptions.hpp"
#include "BEProtocolV2SimpleClient.hpp"

namespace smsc {
namespace mcisme {

BEProtocolV2SimpleClient::BEProtocolV2SimpleClient(const std::string& host, int port)
  : AdvertisingImpl(host, port)
{}

void
BEProtocolV2SimpleClient::parseBannerResponse(const char* packet, uint32_t total_packet_len,
                                              uint32_t* transaction_id, std::string* banner,
                                              int32_t* banner_id, uint32_t* owner_id,
                                              uint32_t* rotator_id, std::string* /*service_name*/)
{
  uint32_t *s = (uint32_t*)packet;
  uint32_t cmdType = ntohl(s[0]);

  if ( cmdType != CMD_BANNER_RSP ) {
    smsc_log_error(_logger, "BEProtocolV2SimpleClient::parseBannerResponse::: incorrect packet type %d", cmdType);
    throw ProtocolError();
  }

  uint32_t offset = CMD_HEADER_SIZE;

  while (offset < total_packet_len) {
    const char* curPtrInBuf = packet + offset;
    uint32_t tag, len;
    offset += extractParamHeader(curPtrInBuf, total_packet_len - offset, &tag, &len);
    curPtrInBuf = packet + offset;
    switch (tag) {
    case TRANSACTION_ID_TAG:
      offset += extractParamValue(curPtrInBuf, total_packet_len - offset, transaction_id);
      break;
    case BANNER_ID_TAG:
      offset += extractParamValue(curPtrInBuf, total_packet_len - offset, reinterpret_cast<uint32_t*>(banner_id));
      break;
    case OWNER_ID_TAG:
      offset += extractParamValue(curPtrInBuf, total_packet_len - offset, owner_id);
      break;
    case ROTATOR_ID_TAG:
      offset += extractParamValue(curPtrInBuf, total_packet_len - offset, rotator_id);
      break;
    case BANNER_BODY_TAG:
      offset += extractParamValue(curPtrInBuf, total_packet_len - offset, banner, len);
      break;
    case CLICK_PRICE_TAG:
      {
        uint32_t clickPrice;
        offset += extractParamValue(curPtrInBuf, total_packet_len - offset, &clickPrice);
      }
      break;
    case CHARSET_TAG:
      {
        uint32_t charset;
        offset += extractParamValue(curPtrInBuf, total_packet_len - offset, &charset);
      }
      break;
    default:
      smsc_log_error(_logger, "BEProtocolV2SimpleClient::parseBannerResponse::: got unknown parameter type 0x%X", tag);
      throw ProtocolError();
    }
  }
  smsc_log_info(_logger, "got BannerResponse: transactionId=%u,bannerId=%d,ownerId=%u,rotatorId=%u",
                *transaction_id, *banner_id, *owner_id, *rotator_id);
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
                                              uint32_t err_code)
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
  req->WriteNetInt32(err_code);

  return totalPacketSize;
}

}}
