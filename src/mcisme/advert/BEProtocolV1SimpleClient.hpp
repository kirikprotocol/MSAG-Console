#ifndef __MCISME_ADVERT_BEPROTOCOLV1SIMPLECLIENT_HPP__
# define __MCISME_ADVERT_BEPROTOCOLV1SIMPLECLIENT_HPP__

# include <mcisme/advert/AdvertisingImpl.h>
# include <core/buffers/TmpBuf.hpp>
# include <util/BufferSerialization.hpp>

namespace smsc {
namespace mcisme {

class BEProtocolV1SimpleClient : public AdvertisingImpl {
public:
  BEProtocolV1SimpleClient(const std::string& host, int port, int timeout);

  static const unsigned int PROTOCOL_VERSION = 1;
protected:
  virtual uint32_t readAdvert(advertising_item* advItem,
                              BannerResponseTrace* bannerRespTrace);

  virtual uint32_t prepareBannerReqCmd(util::SerializationBuffer* req /* буфер*/,
                                       BannerRequest* par /*параметры запроса банера*/);

  uint32_t prepareErrorInfoCmd(util::SerializationBuffer* req /* буфер*/,
                               const BannerRequest& par /*параметры запроса банера*/);

  uint32_t prepareRollbackBannerCmd(util::SerializationBuffer* req,
                                    const BannerRequest& par);

  uint32_t readPacket(core::buffers::TmpBuf<char, MAX_PACKET_LEN>* buf);

  int extractBanner(core::buffers::TmpBuf<char, MAX_PACKET_LEN>& incomingPacketBuf,
                    std::string* banner, uint32_t* bannerId);

  virtual void sendErrorInfo(const BannerRequest& banReq,
                             int rc,
                             const std::string& where);

  static const unsigned int TRANSACTION_ID_LV_SIZE = sizeof(uint32_t) + sizeof(uint32_t);
  static const unsigned int BANNER_ID_LEN_SIZE = sizeof(uint32_t);
  static const unsigned int BANNER_ID_LV_SIZE = BANNER_ID_LEN_SIZE + sizeof(uint32_t);
  static const unsigned int BANNER_LEN_SIZE = sizeof(uint32_t);
  
  static const unsigned int BANNER_OFFSET_IN_PACKET = CMD_HEADER_SIZE + TRANSACTION_ID_LV_SIZE;
  static const unsigned int BANNER_ID_OFFSET_IN_PACKET = CMD_HEADER_SIZE + TRANSACTION_ID_LV_SIZE;
  static const unsigned int GET_BANNER_REQ_LEN = 40; // общая длина фиксированных параметров в getBannerReq
  static const unsigned int ROLLBACK_BANNER_LEN = 16; // total length of RollbackBanner command

  enum 
  {
    CMD_BANNER_REQ = 0x01,    // запрос от клиента к серверу
    CMD_BANNER_RSP = 0x02,        // ответ  от сервера
    CMD_GET_BANNER_WITH_ID_REQ = 0x05,    // запрос от клиента к серверу
    CMD_GET_BANNER_WITH_ID_RSP = 0x06,    // запрос от клиента к серверу
    CMD_ERR_INFO = 0x100,   // пакет от клиента к серверу с информацией об ошибке
    CMD_ROLLBACK_BANNER = 0x101
  };

  enum {
    BANNER_RESPONSE_TIMEOUT = 1,
    BANNER_OTHER_ERROR = 100
  };

};

}}

#endif
