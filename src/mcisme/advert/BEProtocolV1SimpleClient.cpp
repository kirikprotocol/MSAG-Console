#include <mcisme/Exceptions.hpp>
#include "BEProtocolV1SimpleClient.hpp"
#include "AdvertErrors.h"

namespace smsc {
namespace mcisme {

BEProtocolV1SimpleClient::BEProtocolV1SimpleClient(const std::string& host, int port, int timeout)
  : AdvertisingImpl(host, port, timeout)
{}

uint32_t
BEProtocolV1SimpleClient::readPacket(core::buffers::TmpBuf<char, MAX_PACKET_LEN>* buf)
{
  buf->SetPos(0);
  uint32_t *s = (uint32_t*)buf->GetCurPtr();

  int len = CMD_HEADER_SIZE + TRANSACTION_ID_LV_SIZE + BANNER_LEN_SIZE;

  // читаем в буфер заголовок
  readFromSocket(buf->GetCurPtr(), len, "BEProtocolV1SimpleClient::readPacket");

  buf->SetPos(len);

  uint32_t word = ntohl(s[0]);
  if (word != CMD_BANNER_RSP)                     // тип должен быть ответом на запрос баннера
  {
    smsc_log_warn(_logger, "BEProtocolV1SimpleClient::readPacket::: incorrect packet type %d", word);
    generateUnrecoveredProtocolError();
  }

  uint32_t pak_len = ntohl(s[1]) + static_cast<uint32_t>(CMD_HEADER_SIZE);    // длина всего пакета
  if (pak_len > MAX_PACKET_LEN)
  {
    smsc_log_warn(_logger, "BEProtocolV1SimpleClient::readPacket::: bad packet length");
    generateUnrecoveredProtocolError();
  }

  readFromSocket(buf->GetCurPtr(), pak_len-len, "BEProtocolV1SimpleClient::readPacket");

  buf->SetPos(pak_len);

  return ntohl(s[3]);                     //  значение TransactID
}

uint32_t
BEProtocolV1SimpleClient::readAdvert(advertising_item* advItem)
{
  smsc_log_debug(_logger, "BEProtocolV1SimpleClient::readAdvert");

  uint32_t gotTransactId;
  core::buffers::TmpBuf<char, MAX_PACKET_LEN> incomingPacketBuf(0);
  try {
    while( (gotTransactId = readPacket(&incomingPacketBuf)) != advItem->TransactID )
      smsc_log_error(_logger, "BEProtocolV1SimpleClient::readAdvert::: wrong transactionId value=[%d], expected value=[%d]", gotTransactId, advItem->TransactID);
  } catch (TimeoutException& ex) {
    smsc_log_error(_logger, "BEProtocolV1SimpleClient::readAdvert::: catched TimeoutException [%s]", ex.what());
    return ERR_ADV_TIMEOUT;
  }

  extractBanner(incomingPacketBuf, &advItem->banReq->banner);
 
  return 0;
}

int
BEProtocolV1SimpleClient::extractBanner(core::buffers::TmpBuf<char, MAX_PACKET_LEN>& incomingPacketBuf,
                                       std::string* banner)
{
  util::SerializationBuffer buf4parsing;
  buf4parsing.setExternalBuffer(incomingPacketBuf.get() + BANNER_OFFSET_IN_PACKET, static_cast<uint32_t>(incomingPacketBuf.GetPos() - BANNER_OFFSET_IN_PACKET));

  buf4parsing.ReadString<uint32_t>(*banner);

  return 0;
}

uint32_t
BEProtocolV1SimpleClient::prepareBannerReqCmd(util::SerializationBuffer* req, BannerRequest* par)
{
  uint32_t totalPacketSize = prepareHeader(CMD_BANNER_REQ, static_cast<uint32_t>(GET_BANNER_REQ_LEN + par->abonent.length() + par->serviceName.length()), req);

  //TransactId
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t))); //Param length
  req->WriteNetInt32(par->getId());   //Param body

  // abonent
  req->WriteNetInt32(static_cast<uint32_t>(par->abonent.length()));
  req->Write(par->abonent.c_str(), par->abonent.length());

  // service_Name
  req->WriteNetInt32(static_cast<uint32_t>(par->serviceName.length()));
  req->Write(par->serviceName.c_str(), par->serviceName.length());

  // transportType
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(par->transportType)));
  req->WriteNetInt32(par->transportType);

  uint32_t bannerLen;
  switch (par->transportType)
  {
  case TRANSPORT_TYPE_SMS : bannerLen = TRANSPORT_LEN_SMS;  break;
  case TRANSPORT_TYPE_USSD: bannerLen = TRANSPORT_LEN_USSD; break;
  case TRANSPORT_TYPE_HTTP: bannerLen = TRANSPORT_LEN_HTTP; break;
  case TRANSPORT_TYPE_MMS : bannerLen = TRANSPORT_LEN_MMS;  break;
  }

  req->WriteNetInt32(static_cast<uint32_t>(sizeof(bannerLen)));
  req->WriteNetInt32(bannerLen);

  //charSet
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(par->charSet)));
  req->WriteNetInt32(par->charSet);

  return totalPacketSize;
}

uint32_t
BEProtocolV1SimpleClient::prepareErrorInfoCmd(util::SerializationBuffer* req, BannerRequest* par,
                                              uint32_t cmdInfo)
{
  uint32_t totalPacketSize = prepareHeader(CMD_ERR_INFO, ERR_INFO_LEN, req);

  //TransactId45
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t))); //Param length
  req->WriteNetInt32(par->getId());   //Param body

  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(cmdInfo);

  return totalPacketSize;
}

void
BEProtocolV1SimpleClient::sendErrorInfo(util::SerializationBuffer& req,
                                        BannerRequest& banReq,
                                        int rc,
                                        const std::string& where)
{
  if (rc == ERR_ADV_TIMEOUT)
  {
    //если таймаут сервера - сообщить серверу об этой ошибке
    int len = prepareErrorInfoCmd(&req, &banReq, ERR_ADV_TIMEOUT);
    writeToSocket(req.getBuffer(), len, where);
  }
}

}}
