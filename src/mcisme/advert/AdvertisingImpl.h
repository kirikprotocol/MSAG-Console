#ifndef __MCISME_ADVERT_ADVERTISINGIMPL_HPP__
# define __MCISME_ADVERT_ADVERTISINGIMPL_HPP__

#include <util/BufferSerialization.hpp>
#include <mcisme/advert/Advertising.h>
#include "core/network/Socket.hpp"

namespace smsc {
namespace mcisme {

// ���� �����������
enum
{
  TRANSPORT_TYPE_SMS = 1,
  TRANSPORT_TYPE_USSD,
  TRANSPORT_TYPE_HTTP,
  TRANSPORT_TYPE_MMS
};
// ������������ ����� ������� �����������
enum
{
  TRANSPORT_LEN_SMS  =  140,
  TRANSPORT_LEN_USSD =  160,
  TRANSPORT_LEN_HTTP = 1024,
  TRANSPORT_LEN_MMS  = 1024
};

#define ERR_INFO_LEN 16             // ����� �����std::vector <char > buf; ������������� ���������� � errInfo

static int CommonTransactID = 0;    // ����� ������� ����������
static Mutex CommTransId_mtx;       // ������� ��� ������� � ��������

#define BANNER_LEN 1024                     // max len �������
#define MAX_PACKET_LEN 1048                 // ������������ ����� ����� ������

#define MAX_WAIT_TIME 120000// (~2 ���) ������������� ���������� ����� ��������������� ����������
#define MIN_WAIT_TIME 500

struct advertising_item
{
  uint32_t TransactID;     // ����� ����������
  BannerRequest* banReq;

  explicit advertising_item(BannerRequest* aBanReq)
  {
    banReq = aBanReq;
    TransactID = banReq->getId();
  }
};

class AdvertisingImpl : public Advertising {
public:
  virtual void init(int connectTimeout=0);
  virtual bool reinit(int connectTimeout=0);

  virtual uint32_t getBanner(const std::string& abonent,
                             const std::string& service_name,
                             uint32_t transport_type, uint32_t char_set,
                             std::string* banner,
                             BannerResponseTrace* banner_resp_trace,
                             size_t max_banner_size);

  virtual void rollbackBanner(uint32_t transactionId,
                              uint32_t bannerId,
                              uint32_t ownerId,
                              uint32_t rotatorId);

  std::string toString() const;
protected:
  std::string _host;
  in_port_t _port;
  uint32_t _timeout;
  bool _isConnected;
  smsc::core::network::Socket _socket;
  static const unsigned int CMD_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint32_t);

  AdvertisingImpl(const std::string& host, int port, int timeout)
    : _host(host), _port(port), _timeout(timeout), _isConnected(false)
  {}

  void writeErrorToLog(char* where, int errCode);

  void readFromSocket(char *dataBuf, int bytesToRead, const std::string& where);
  void writeToSocket(const void* buf, int bufSize, const std::string& where);

  uint32_t getBanner(BannerRequest& banReq, BannerResponseTrace* bannerRespTrace);

  int sendRequestAndGetResponse(advertising_item* advItem,
                                util::SerializationBuffer* req,
                                uint32_t req_len,
                                BannerResponseTrace* bannerRespTrace);

  virtual uint32_t readAdvert(advertising_item* advItem, BannerResponseTrace* bannerRespTrace) = 0;

  virtual uint32_t prepareHeader(uint32_t cmndType, uint32_t reqBodyLen, util::SerializationBuffer* req);

  virtual uint32_t prepareBannerReqCmd(util::SerializationBuffer* req /* �����*/,
                                       BannerRequest* par /*��������� ������� ������*/) = 0;

  virtual void sendErrorInfo(const BannerRequest& banReq,
                             int rc,
                             const std::string& where) = 0;

  void generateUnrecoveredProtocolError();
};

}}

#endif
