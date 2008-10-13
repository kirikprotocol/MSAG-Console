#include "AdvertisingImpl.h"
#include <string.h>
#include <util/Exception.hpp>
#include <util/BufferSerialization.hpp>

#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>

#include <mcisme/Exceptions.hpp>

namespace smsc {
namespace mcisme {

SimpleAdvertisingClient::SimpleAdvertisingClient(const std::string& host, int port, int timeout)
  : _logger(Logger::getInstance("scag.advert.Advertising")),
    _host(host), _port(port), _timeout(timeout)
{}

void
SimpleAdvertisingClient::init(int connectTimeout)
{
  if ( _socket.Init(_host.c_str(), _port, _timeout) )
    throw util::Exception("AdvertisingImpl::init::: can't init socket for host=[%s],port=[%d] - '%s'", _host.c_str(), _port, strerror(errno));

  _socket.setConnectTimeout(connectTimeout);

  if ( _socket.Connect() )
    throw util::Exception("AdvertisingImpl::init::: can't establish connection to host=[%s],port=[%d] - '%s'", _host.c_str(), _port, strerror(errno));
}

void
SimpleAdvertisingClient::reinit(int connectTimeout)
{
  _socket.setConnectTimeout(connectTimeout);

  if ( _socket.Connect() )
    smsc_log_error(_logger, "AdvertisingImpl::reinit::: can't establish connection to host=[%s],port=[%d] - %s", _host.c_str(), _port, strerror(errno));
}

void SimpleAdvertisingClient::CheckBannerRequest(BannerRequest& banReq, int async)
{
  smsc_log_debug(_logger,"SimpleAdvertisingClient::CheckBannerRequest() abonent = %s, serviceName = %s, transportType = %d, charSet = %d, async = %d",
                 banReq.abonent.c_str(), banReq.serviceName.c_str(), banReq.transportType, banReq.charSet, async);

  __require__(banReq.abonent.length() > 5 && banReq.abonent.length() < 21);
}

uint32_t
SimpleAdvertisingClient::getBanner(const std::string& abonent,
                                   const std::string& serviceName,
                                   uint32_t transportType, uint32_t charSet,
                                   std::string &banner)
{
  BannerRequest req(abonent, serviceName, transportType, charSet);
  uint32_t rc = getBanner(req);
  banner = req.banner;
  return  rc;
}

uint32_t
SimpleAdvertisingClient::getBanner(BannerRequest& banReq)
{
  CheckBannerRequest(banReq);

  advertising_item curAdvItem(&banReq);

  util::SerializationBuffer req;
  // заполнение буфера протокольной команды
  uint32_t len = PrepareCmnd(&req, &banReq, CMD_BANNER_REQ);

  int rc = sendRequestAndGetResponse(&curAdvItem, &req, len);
  if (rc != 0)
  {
    WriteErrorToLog((char*)"SimpleAdvertisingClient::ServBanner", rc);
    if (rc == ERR_ADV_TIMEOUT)
    {
      //если таймаут сервера - сообщить серверу об этой ошибке
      len = PrepareCmnd(&req, &banReq, CMD_ERR_INFO, ERR_ADV_TIMEOUT);
      writeToSocket(req.getBuffer(), len, "SimpleAdvertisingClient::getBanner:::");
    }
  }

  return rc;
}

int
SimpleAdvertisingClient::sendRequestAndGetResponse(advertising_item* advItem, SerializationBuffer* req, uint32_t req_len)
{
  writeToSocket(req->getBuffer(), req_len, "SimpleAdvertisingClient::sendRequestAndGetResponse");

  return readAdvert(advItem);
}

void
SimpleAdvertisingClient::writeToSocket(const void* dataBuf, int dataSz, const std::string& where)
{
  if ( _socket.WriteAll((char *)dataBuf, dataSz) < 0 ) {
    std::string errMsg = where + " socket write error [" + strerror(errno) + "]";
    smsc_log_warn(_logger, errMsg);
    throw NetworkException(errMsg.c_str());
  }
}

void
SimpleAdvertisingClient::readFromSocket(char *dataBuf, int bytesToRead, const std::string& where)
{
  int rd=0,res;
  smsc_log_debug(_logger, "SimpleAdvertisingClient::readFromSocket: _timeout=%d, bytesToRead=%d", _timeout, bytesToRead);
  while(rd<bytesToRead)
  {
    if ( _timeout > 0 ) {
      res = _socket.canRead(_timeout);
      if ( res < 0 )
        throw NetworkException("SimpleAdvertisingClient::readFromSocket::: read socket error = %d", errno);
      else if ( !res )
        throw NetworkException("SimpleAdvertisingClient::readFromSocket::: read timeout was expired (timeout value=%d)", _timeout);
    }
    res=_socket.Read(dataBuf+rd,bytesToRead-rd);
    if(res<0)
      throw NetworkException("SimpleAdvertisingClient::readFromSocket::: read socket error = %d", errno);
    else if (!res)
      throw NetworkException("SimpleAdvertisingClient::readFromSocket::: connection closed by remote side");
    smsc_log_debug(_logger, "SimpleAdvertisingClient::readFromSocket: _socket.read returned %d",res);
    rd+=res;
  }
}

uint32_t
SimpleAdvertisingClient::readPacket(core::buffers::TmpBuf<char, MAX_PACKET_LEN>* buf)
{
  buf->SetPos(0);
  uint32_t *s = (uint32_t*)buf->GetCurPtr();

  int len = CMD_HEADER + TRANSACTION_ID_LV_SIZE + BANNER_LEN_SIZE;

  // читаем в буфер заголовок
  readFromSocket(buf->GetCurPtr(), len, "SimpleAdvertisingClient::readPacket");

  buf->SetPos(len);

  uint32_t word = ntohl(s[0]);
  if (word != CMD_BANNER_RSP)                     // тип должен быть ответом на запрос баннера
  {
    smsc_log_error(_logger, "SimpleAdvertisingClient::readPacket::: incorrect packet type %d", word);
    throw std::exception();
  }

  uint32_t pak_len = ntohl(s[1]) + static_cast<uint32_t>(CMD_HEADER);    // длина всего пакета
  if (pak_len > MAX_PACKET_LEN)
  {
    smsc_log_warn(_logger, "SimpleAdvertisingClient::readPacket::: bad packet length");
    throw std::exception();
  }

  readFromSocket(buf->GetCurPtr(), pak_len-len, "SimpleAdvertisingClient::readPacket");

  buf->SetPos(pak_len);

  return ntohl(s[3]);                     //  значение TransactID
}

uint32_t
SimpleAdvertisingClient::readAdvert(advertising_item* advItem)
{
  smsc_log_debug(_logger, "SimpleAdvertisingClient::readAdvert");

  int rc = 0;

  try {
    uint32_t gotTransactId;
    TmpBuf<char, MAX_PACKET_LEN> incomingPacketBuf(0);
    while( (gotTransactId = readPacket(&incomingPacketBuf)) != advItem->TransactID )
      smsc_log_error(_logger, "SimpleAdvertisingClient::readAdvert::: wrong transactionId value=[%d], expected value=[%d]", gotTransactId, advItem->TransactID);

    rc = extractBanner(incomingPacketBuf, &advItem->banReq->banner);
  } catch(util::Exception& ex) {
    smsc_log_error(_logger, "SimpleAdvertisingClient::readAdvert::: catched exception [%s]", ex.what());
  }

  return rc;
}

int
SimpleAdvertisingClient::extractBanner(TmpBuf<char, MAX_PACKET_LEN>& incomingPacketBuf,
                                       std::string* banner)
{
  util::SerializationBuffer buf4parsing;
  buf4parsing.setExternalBuffer(incomingPacketBuf.get() + BANNER_OFFSET_IN_PACKET, static_cast<uint32_t>(incomingPacketBuf.GetPos() - BANNER_OFFSET_IN_PACKET));

  buf4parsing.ReadString<uint32_t>(*banner);

  return 0;
}

//------------------------------------------------------------------------------
// запись в журнал сообщения об ошибке
//
void SimpleAdvertisingClient::WriteErrorToLog(char* where, int errCode)
{
  switch(errCode)
  {
  case ERR_ADV_SOCKET     : smsc_log_error(_logger, "%s, server hangs", where); break;
  case ERR_ADV_SOCK_WRITE   : smsc_log_error(_logger, "%s, socket closed", where); break;
  case ERR_ADV_PACKET_TYPE  : smsc_log_error(_logger, "%s, bad packet type", where); break;
  case ERR_ADV_PACKET_LEN   : smsc_log_error(_logger, "%s, bad packet length", where); break;
  case ERR_ADV_PACKET_MEMBER  : smsc_log_error(_logger, "%s, bad packet member", where); break;
  case ERR_ADV_QUEUE_FULL     : smsc_log_error(_logger, "%s, queue is overloaded", where); break;

  default           : smsc_log_error(_logger, "%s, error %d", where, errCode);
  }
}

//------------------------------------------------------------------------------
// заполнение буфера протокольной команды
//
uint32_t SimpleAdvertisingClient::PrepareCmnd(SerializationBuffer* req, BannerRequest* par,
                                      uint32_t cmndType, uint32_t cmdInfo)
{
  uint32_t req_len;

  switch(cmndType)   // длина  пакета  для каждой из команд клиента
  {
  case CMD_BANNER_REQ:
    req_len = static_cast<uint32_t>(GET_BANNER_REQ_LEN + par->abonent.length() + par->serviceName.length());
    break;

  case CMD_ERR_INFO:
    req_len = ERR_INFO_LEN;
    break;
  }

  req->resize(static_cast<uint32_t>(req_len + CMD_HEADER));
  req->setPos(0);

  //command
  req->WriteNetInt32(cmndType);       //Command Type
  req->WriteNetInt32(req_len);      //Command length

  //TransactId45
  req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t))); //Param length
  req->WriteNetInt32(par->getId());   //Param body

  if(cmndType == CMD_ERR_INFO)
  {
    req->WriteNetInt32(static_cast<uint32_t>(sizeof(uint32_t)));
    req->WriteNetInt32(cmdInfo);
  }
  else
  {
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

  } //cmndType != CMD_ERR_INFO

  return static_cast<uint32_t>(req_len + CMD_HEADER);//возвращается ОБЩАЯ длина пакета
}

} // advert
} // scag
