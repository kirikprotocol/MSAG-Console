#ifndef __MCISME_ADVERT_BEPROTOCOLV2SIMPLECLIENT_HPP__
# define __MCISME_ADVERT_BEPROTOCOLV2SIMPLECLIENT_HPP__

# include <mcisme/advert/AdvertisingImpl.h>
# include <core/buffers/TmpBuf.hpp>
# include <util/BufferSerialization.hpp>
# include <util/Exception.hpp>

namespace smsc {
namespace mcisme {

class BEProtocolV2SimpleClient : public AdvertisingImpl {
public:
  BEProtocolV2SimpleClient(const std::string& host, int port, int timeout);

  static const unsigned int PROTOCOL_VERSION = 2;
protected:
  virtual uint32_t readAdvert(advertising_item* advItem,
                              BannerResponseTrace* bannerRespTrace);

  virtual uint32_t prepareBannerReqCmd(util::SerializationBuffer* req /* �����*/,
                                       BannerRequest* par /*��������� ������� ������*/);

  uint32_t prepareErrorInfoCmd(util::SerializationBuffer* req /* �����*/,
                               const BannerRequest& par /*��������� ������� ������*/,
                               uint32_t errCode);

  uint32_t readPacket(char* buf, size_t bufSize);

  int extractBanner(core::buffers::TmpBuf<char, MAX_PACKET_LEN>& incomingPacketBuf,
                    std::string* banner);

  void parseBannerResponse(const char* packet, uint32_t totalPacketLen,
                           uint32_t* gotTransactId, std::string* banner,
                           uint32_t* bannerId, uint32_t* ownerId,
                           uint32_t* rotatorId);

  uint32_t extractParamValue(const char* curPtrInBuf, uint32_t restSizeOfBuff,
                             uint32_t* value);

  uint32_t extractParamValue(const char* curPtrInBuf, uint32_t restSizeOfBuff,
                             std::string* value, uint32_t paramLen);

  uint32_t extractParamHeader(const char* curPtrInBuf,
                              uint32_t restSizeOfBuff,
                              uint32_t* tag,
                              uint32_t* len);


  virtual void sendErrorInfo(const BannerRequest& banReq,
                             int rc,
                             const std::string& where);

  static const unsigned int PARAM_HDR_SZ = sizeof(uint32_t) + sizeof(uint32_t);
  // ����� ����� ������������� ���������� � getBannerReq
  static const unsigned int GET_BANNER_REQ_BODY_LEN =
    PARAM_HDR_SZ + sizeof(uint32_t) + // transaction id
    PARAM_HDR_SZ + sizeof(uint32_t) + // banner id
    PARAM_HDR_SZ + sizeof(uint32_t) + // owner id
    PARAM_HDR_SZ + sizeof(uint32_t) + // rotator id
    PARAM_HDR_SZ + // size of parameter header for abonent
    PARAM_HDR_SZ + sizeof(uint32_t) + // banner max len
    PARAM_HDR_SZ + sizeof(uint32_t) + // charset
    PARAM_HDR_SZ + // size of parameter header for service name
    PARAM_HDR_SZ + sizeof(uint32_t);// transport type

  static const unsigned int CMD_ROLLBACK_REQ_BODY_LEN =
    PARAM_HDR_SZ + sizeof(uint32_t) + // transaction id
    PARAM_HDR_SZ + sizeof(uint32_t) + // banner id
    PARAM_HDR_SZ + sizeof(uint32_t) + // owner id
    PARAM_HDR_SZ + sizeof(uint32_t) + // rotator id
    PARAM_HDR_SZ + sizeof(uint32_t); // error code

  static const unsigned int GET_BANNER_RESP_BODY_LEN = 
    PARAM_HDR_SZ + sizeof(uint32_t) + // transaction id
    PARAM_HDR_SZ + sizeof(uint32_t) + // banner id
    PARAM_HDR_SZ + sizeof(uint32_t) + // owner id
    PARAM_HDR_SZ + sizeof(uint32_t) + // rotator id
    PARAM_HDR_SZ + sizeof(uint32_t); // click price

  enum {
    CMD_BANNER_REQ = 0x01,      // ������ �� ������� � �������
    CMD_BANNER_RSP = 0x02,          // �����  �� �������
    CMD_ROLLBACK_REQ = 0x101 // ����� �� ������� � ������� � ����������� �� ������
  };

  enum {
    TRANSACTION_ID_TAG = 0x01, ABONENT_TAG = 0x02, SERVICE_NAME_TAG = 0x03, TRANSPORT_TYPE_TAG =0x04,
    BANNER_MAX_LEN_TAG = 0x05, CHARSET_TAG = 0x06, BANNER_ID_TAG = 0x07, BANNER_BODY_TAG = 0x09,
    EXT_INFO_BODY_TAG = 0x0A, CLICK_PRICE_TAG = 0x0B, FLAGS_TAG = 0x0C, ERROR_CODE_TAG = 0x0D, 
    ROTATOR_ID_TAG = 0x0E, OWNER_ID_TAG = 0x0F
  };

  enum {
    BANNER_RESPONSE_TIMEOUT = -100,
    BANNER_OTHER_ERROR = -101
  };

  class ProtocolError : public smsc::util::Exception {
  public:
  };
};

}}

#endif
