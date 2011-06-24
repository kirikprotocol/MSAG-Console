#ifndef __MCISME_ADVERT_ADVERTISINGIMPL_HPP__
# define __MCISME_ADVERT_ADVERTISINGIMPL_HPP__

# include "util/BufferSerialization.hpp"
# include "core/buffers/RefPtr.hpp"
# include "core/network/Socket.hpp"
# include "core/synchronization/Mutex.hpp"
# include "mcisme/AbntAddr.hpp"
# include "mcisme/advert/Advertising.h"
# include "mcisme/advert/AdvertErrors.h"

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

  virtual uint32_t sendBannerRequest(const std::string& abonent,
                                     const std::string& service_name,
                                     uint32_t transport_type, uint32_t char_set,
                                     uint32_t max_banner_size, MCEventOut* mc_event_out);

  virtual void rollbackBanner(uint32_t transactionId,
                              uint32_t bannerId,
                              uint32_t ownerId,
                              uint32_t rotatorId);

  std::string toString() const;

  virtual banner_read_stat readAdvert(std::string* banner, BannerResponseTrace* banner_resp_trace) = 0;

  virtual void sendErrorInfo(const BannerRequest& banReq,
                             int rc) = 0;

  int getSocketFd() const {
    return _socket.getSocket();
  }

protected:
  std::string _host;
  in_port_t _port;
  bool _isConnected;
  core::network::Socket _socket;
  static const unsigned int CMD_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint32_t);

  AdvertisingImpl(const std::string& host, int port)
    : _host(host), _port(port), _isConnected(false)
  {}

  void writeErrorToLog(char* where, int errCode);

  int readFromSocket(char *dataBuf, int bytesToRead, const std::string& where);
  void writeToSocket(const void* buf, int bufSize, const std::string& where);

  void sendBannerRequest(BannerRequest* banReq);

  virtual uint32_t prepareHeader(uint32_t cmndType, uint32_t reqBodyLen, util::SerializationBuffer* req);

  virtual uint32_t prepareBannerReqCmd(util::SerializationBuffer* req /* �����*/,
                                       BannerRequest* par /*��������� ������� ������*/) = 0;

  void generateUnrecoveredProtocolError();
};

typedef core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex> AdvertImplRefPtr;

}}

#endif
