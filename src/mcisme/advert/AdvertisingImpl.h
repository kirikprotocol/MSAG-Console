#ifndef __MCISME_ADVERT_ADVERTISINGIMPL_HPP__
# define __MCISME_ADVERT_ADVERTISINGIMPL_HPP__

# include "util/BufferSerialization.hpp"
# include "core/buffers/RefPtr.hpp"
# include "core/network/Socket.hpp"
# include "core/synchronization/Mutex.hpp"
# include "mcisme/AbntAddr.hpp"
# include "mcisme/advert/Advertising.h"
# include "mcisme/advert/AdvertErrors.h"
# include "mcisme/Exceptions.hpp"

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
  virtual void init(int connect_timeout=0);
  virtual bool reinit(int connect_timeout=0);

  virtual uint32_t sendBannerRequest(const std::string& abonent,
                                     const std::string& service_name,
                                     uint32_t transport_type, uint32_t char_set,
                                     uint32_t max_banner_size, MCEventOut* mc_event_out);

  virtual void rollbackBanner(uint32_t transaction_id,
                              uint32_t banner_id,
                              uint32_t owner_id,
                              uint32_t rotator_id,
                              const std::string& service_name);

  std::string toString() const;

  banner_read_stat readAdvert(std::string* banner, BannerResponseTrace* banner_resp_trace);

  void sendErrorInfo(const BannerRequest& ban_req, int rc);

  int getSocketFd() const {
    return _socket.getSocket();
  }

protected:
  AdvertisingImpl(const std::string& host, int port)
    : _host(host), _port(port), _isConnected(false),
      _pos(0), _totalPacketLen(0)
  {}

  void writeErrorToLog(char* where, int err_code);

  int readFromSocket(char *dataBuf, int bytes_to_read, const std::string& where);
  void writeToSocket(const void* buf, int buf_size, const std::string& where);

  void sendBannerRequest(BannerRequest* ban_req);

  virtual uint32_t prepareHeader(uint32_t cmd_type, uint32_t req_body_len, util::SerializationBuffer* req);

  virtual uint32_t prepareBannerReqCmd(util::SerializationBuffer* req /* �����*/,
                                       BannerRequest* par /*��������� ������� ������*/) = 0;

  virtual uint32_t prepareErrorInfoCmd(util::SerializationBuffer* req, const BannerRequest& par,
                                       uint32_t err_code) = 0;

  virtual void parseBannerResponse(const char* packet, uint32_t total_packet_len,
                                   uint32_t* transaction_id, std::string* banner,
                                   int32_t* banner_id, uint32_t* owner_id,
                                   uint32_t* rotator_id, std::string *service_name) = 0;

  void generateUnrecoveredProtocolError();

  uint32_t readPacket();

  uint32_t extractParamValue(const char* cur_ptr_in_buf, uint32_t rest_size_of_buff,
                             uint32_t* value);

  uint32_t extractParamValue(const char* cur_ptr_in_buf, uint32_t rest_size_of_buff,
                             std::string* value, uint32_t param_len);

  uint32_t
  extractParamHeader(const char* cur_ptr_in_buf, uint32_t rest_size_of_buff,
                     uint32_t* tag, uint32_t* len);

  class ProtocolError : public util::Exception {
  public:
  };

  std::string _host;
  in_port_t _port;
  bool _isConnected;
  int _pos;
  uint32_t _totalPacketLen;
  core::network::Socket _socket;
  char _buf[MAX_PACKET_LEN];

  static const unsigned int CMD_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint32_t);

  static const unsigned int PARAM_HDR_SZ = sizeof(uint32_t) + sizeof(uint32_t);

  enum {
    BANNER_RESPONSE_TIMEOUT = -100,
    BANNER_OTHER_ERROR = -101
  };
};

typedef core::buffers::RefPtr<AdvertisingImpl, core::synchronization::Mutex> AdvertImplRefPtr;

}}

#endif
