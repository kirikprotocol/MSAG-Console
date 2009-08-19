#include <mcisme/Exceptions.hpp>
#include "BEProtocolV1SimpleClient.hpp"
#include "AdvertErrors.h"

namespace smsc {
namespace mcisme {

BEProtocolV1SimpleClient::BEProtocolV1SimpleClient(const std::string& host, int port,
                                                   int timeout, bool use_get_banner_with_id_req)
  : AdvertisingImpl(host, port, timeout), _waitingForGetBannerWithIdRSP(use_get_banner_with_id_req)
{}

uint32_t
BEProtocolV1SimpleClient::readPacket(core::buffers::TmpBuf<char, MAX_PACKET_LEN>* buf)
{
  buf->SetPos(0);
  uint32_t *s = (uint32_t*)buf->GetCurPtr();

  int len = CMD_HEADER_SIZE;

  readFromSocket(buf->GetCurPtr(), len, "BEProtocolV1SimpleClient::readPacket");

  buf->SetPos(len);

  uint32_t word = ntohl(s[0]);
  if (word != CMD_GET_BANNER_WITH_ID_RSP && word != CMD_BANNER_RSP )
  {
    smsc_log_warn(_logger, "BEProtocolV1SimpleClient::readPacket::: incorrect packet type %d, expected packet type=%d or packet type=%d",
                  word, CMD_GET_BANNER_WITH_ID_RSP, CMD_BANNER_RSP);
    generateUnrecoveredProtocolError();
  }

  uint32_t pak_len = ntohl(s[1]) + static_cast<uint32_t>(CMD_HEADER_SIZE);
  if (pak_len > MAX_PACKET_LEN)
  {
    smsc_log_warn(_logger, "BEProtocolV1SimpleClient::readPacket::: too large packet=%d, max expected packet size=%d",
                  pak_len, MAX_PACKET_LEN);
    generateUnrecoveredProtocolError();
  }

  readFromSocket(buf->GetCurPtr(), pak_len-len, "BEProtocolV1SimpleClient::readPacket");

  buf->SetPos(pak_len);

  return ntohl(s[3]);
}

uint32_t
BEProtocolV1SimpleClient::readAdvert(advertising_item* advItem,
                                     BannerResponseTrace* bannerRespTrace)
{
  smsc_log_debug(_logger, "BEProtocolV1SimpleClient::readAdvert");

  uint32_t gotTransactId;
  core::buffers::TmpBuf<char, MAX_PACKET_LEN> incomingPacketBuf(0);
  try {
    while( (gotTransactId = readPacket(&incomingPacketBuf)) != advItem->TransactID )
      smsc_log_error(_logger, "BEProtocolV1SimpleClient::readAdvert::: wrong transactionId value=[%d], expected value=[%d]", gotTransactId, advItem->TransactID);
  } catch (TimeoutException& ex) {
    smsc_log_error(_logger, "BEProtocolV1SimpleClient::readAdvert::: caught TimeoutException [%s]", ex.what());
    return ERR_ADV_TIMEOUT;
  }

  bannerRespTrace->transactionId = advItem->TransactID;
  if ( _waitingForGetBannerWithIdRSP )
    extractBanner(incomingPacketBuf, &advItem->banReq->banner, &bannerRespTrace->bannerId);
  else {
    extractBanner(incomingPacketBuf, &advItem->banReq->banner);
    bannerRespTrace->bannerIdIsNotUsed = true;
  }
  return 0;
}

int
BEProtocolV1SimpleClient::extractBanner(core::buffers::TmpBuf<char, MAX_PACKET_LEN>& incomingPacketBuf,
                                        std::string* banner, uint32_t* bannerId)
{
  util::SerializationBuffer buf4parsing;
  buf4parsing.setExternalBuffer(incomingPacketBuf.get() + BANNER_ID_OFFSET_IN_PACKET, static_cast<uint32_t>(incomingPacketBuf.GetPos() - BANNER_ID_OFFSET_IN_PACKET));

  if ( _waitingForGetBannerWithIdRSP && bannerId ) {
    uint32_t bannerIdParamLen = buf4parsing.ReadNetInt32();
    if ( bannerIdParamLen != BANNER_ID_LEN_SIZE ) {
      smsc_log_warn(_logger, "BEProtocolV1SimpleClient::extractBanner::: bad banner id length (%d), expected (%d)", bannerIdParamLen, BANNER_ID_LEN_SIZE);
      generateUnrecoveredProtocolError();
    }

    *bannerId = buf4parsing.ReadNetInt32();
  }

  buf4parsing.ReadString<uint32_t>(*banner);

  return 0;
}

uint32_t
BEProtocolV1SimpleClient::prepareBannerReqCmd(util::SerializationBuffer* req, BannerRequest* par)
{
  uint32_t totalPacketSize;
  if ( _waitingForGetBannerWithIdRSP )
    totalPacketSize = prepareHeader(CMD_GET_BANNER_WITH_ID_REQ, static_cast<uint32_t>(GET_BANNER_REQ_LEN + par->abonent.length() + par->serviceName.length()), req);
  else
    totalPacketSize = prepareHeader(CMD_BANNER_REQ, static_cast<uint32_t>(GET_BANNER_REQ_LEN + par->abonent.length() + par->serviceName.length()), req);

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

  req->WriteNetInt32(static_cast<uint32_t>(sizeof(par->maxBannerSize)));
  req->WriteNetInt32(par->maxBannerSize);
  //charSet
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(par->charSet)));

  if ( !_waitingForGetBannerWithIdRSP && par->charSet == ASCII_TRANSLIT )
    throw BE_v0_UnsupportedCharsetException("BEProtocolV1SimpleClient::prepareBannerReqCmd::: char set=ASCII_TRANSLIT isn't supported in BannerRequest to BE version 0");
  req->WriteNetInt32(par->charSet);

  return totalPacketSize;
}

uint32_t
BEProtocolV1SimpleClient::prepareErrorInfoCmd(util::SerializationBuffer* req,
                                              const BannerRequest& par)
{
  uint32_t totalPacketSize = prepareHeader(CMD_ERR_INFO, ERR_INFO_LEN, req);

  //TransactId
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t))); //Param length
  req->WriteNetInt32(par.getId());   //Param body

  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(BANNER_RESPONSE_TIMEOUT);

  return totalPacketSize;
}

uint32_t
BEProtocolV1SimpleClient::prepareRollbackBannerCmd(util::SerializationBuffer* req,
                                                   const BannerRequest& par)
{
  uint32_t totalPacketSize = prepareHeader(CMD_ROLLBACK_BANNER, ROLLBACK_BANNER_LEN, req);

  //bannerId
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t))); //Param length
  req->WriteNetInt32(par.bannerId);   //Param body

  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
  req->WriteNetInt32(BANNER_OTHER_ERROR);

  return totalPacketSize;
}

void
BEProtocolV1SimpleClient::sendErrorInfo(const BannerRequest& banReq,
                                        int rc,
                                        const std::string& where)
{
  util::SerializationBuffer req;
  int len;
  if ( rc == ERR_ADV_TIMEOUT )
    len = prepareErrorInfoCmd(&req, banReq);
  else if ( rc == ERR_ADV_OTHER )
    len = prepareRollbackBannerCmd(&req, banReq);
  else
    throw util::Exception("BEProtocolV1SimpleClient::sendErrorInfo::: unknown error reason value=[%d]", rc);

  writeToSocket(req.getBuffer(), len, where);
}

}}
