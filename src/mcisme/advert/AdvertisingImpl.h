#ifndef __MCISME_ADVERT_ADVERTISINGIMPL_HPP__
# define __MCISME_ADVERT_ADVERTISINGIMPL_HPP__

# include <util/BufferSerialization.hpp>
# include <mcisme/advert/Advertising.h>
# include <mcisme/Socket.hpp>

namespace smsc {
namespace mcisme {

// типы транспортов
enum 
{
  TRANSPORT_TYPE_SMS = 1,
  TRANSPORT_TYPE_USSD,
  TRANSPORT_TYPE_HTTP,
  TRANSPORT_TYPE_MMS
};
// максимальные длины пакетов транспортов
enum 
{
  TRANSPORT_LEN_SMS  =  140,
  TRANSPORT_LEN_USSD =  160,
  TRANSPORT_LEN_HTTP = 1024,
  TRANSPORT_LEN_MMS  = 1024
};

#define ERR_INFO_LEN 16             // общая длинаstd::vector <char > buf; фиксированных параметров в errInfo

static int CommonTransactID = 0;    // общий счетчик транзакций
static Mutex CommTransId_mtx;       // мьютекс для доступа к счетчику

#define BANNER_LEN 1024                     // max len баннера
#define MAX_PACKET_LEN 1048                 // максимальная длина всего пакета

#define MAX_WAIT_TIME 120000// (~2 мин) максимальныйе промежуток между восстановлением соединения
#define MIN_WAIT_TIME 500

struct advertising_item
{
  uint32_t TransactID;     // номер транзакции
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
                             const std::string& serviceName,
                             uint32_t transportType, uint32_t charSet,
                             std::string &banner);

  std::string toString() const;
protected:
  std::string _host;
  in_port_t _port;
  uint32_t _timeout;
  bool _isConnected;
  Socket _socket;
  static const unsigned int CMD_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint32_t);

  AdvertisingImpl(const std::string& host, int port, int timeout)
    : _host(host), _port(port), _timeout(timeout), _isConnected(false)
  {}

  void writeErrorToLog(char* where, int errCode);

  void readFromSocket(char *dataBuf, int bytesToRead, const std::string& where);
  void writeToSocket(const void* buf, int bufSize, const std::string& where);

  void checkBannerRequest(BannerRequest& banReq, int async = 0);

  uint32_t getBanner(BannerRequest& banReq);

  int sendRequestAndGetResponse(advertising_item* advItem,
                                util::SerializationBuffer* req,
                                uint32_t req_len);

  virtual uint32_t readAdvert(advertising_item* advItem) = 0;

  virtual uint32_t prepareHeader(uint32_t cmndType, uint32_t reqBodyLen, util::SerializationBuffer* req);

  virtual uint32_t prepareBannerReqCmd(util::SerializationBuffer* req /* буфер*/,
                               BannerRequest* par /*параметры запроса банера*/) = 0;

  virtual uint32_t prepareErrorInfoCmd(util::SerializationBuffer* req /* буфер*/,
                               BannerRequest* par /*параметры запроса банера*/,
                               uint32_t cmdInfo) = 0;

  virtual void sendErrorInfo(util::SerializationBuffer& req,
                             BannerRequest& banReq,
                             int rc,
                             const std::string& where) = 0;

  void generateUnrecoveredProtocolError();
};

}}

#endif
