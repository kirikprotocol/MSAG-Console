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
  virtual uint32_t readAdvert(advertising_item* advItem);

  virtual uint32_t prepareBannerReqCmd(util::SerializationBuffer* req /* �����*/,
                                       BannerRequest* par /*��������� ������� ������*/);

  virtual uint32_t prepareErrorInfoCmd(util::SerializationBuffer* req /* �����*/,
                                       BannerRequest* par /*��������� ������� ������*/,
                                       uint32_t cmdInfo);

  uint32_t readPacket(core::buffers::TmpBuf<char, MAX_PACKET_LEN>* buf);

  int extractBanner(core::buffers::TmpBuf<char, MAX_PACKET_LEN>& incomingPacketBuf,
                    std::string* banner);

  virtual void sendErrorInfo(util::SerializationBuffer& req,
                             BannerRequest& banReq,
                             int rc,
                             const std::string& where);

  static const unsigned int TRANSACTION_ID_LV_SIZE = sizeof(uint32_t) + sizeof(uint32_t);
  static const unsigned int BANNER_LEN_SIZE = sizeof(uint32_t);
  static const unsigned int BANNER_OFFSET_IN_PACKET = CMD_HEADER_SIZE + TRANSACTION_ID_LV_SIZE;
  static const unsigned int GET_BANNER_REQ_LEN = 40; // ����� ����� ������������� ���������� � getBannerReq

  enum 
  {
    CMD_BANNER_REQ = 0x01,    // ������ �� ������� � �������
    CMD_BANNER_RSP = 0x02,        // �����  �� �������
    CMD_ERR_INFO = 0x100   // ����� �� ������� � ������� � ����������� �� ������
  };
};

}}

#endif
