#ifndef MCISME_ADVERT_ADVERTISINGIMPL_CLIENT
#define MCISME_ADVERT_ADVERTISINGIMPL_CLIENT

#include "AdvertErrors.h"
#include "Advertising.h"

#include <logger/Logger.h>

#include <core/buffers/Array.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/TmpBuf.hpp>

#include <core/network/Socket.hpp>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Event.hpp>

#include <util/BufferSerialization.hpp>
#include <core/network/Socket.hpp>

#include <util/debug.h>

using namespace smsc::core::buffers;
using namespace smsc::core::network;
using namespace smsc::core::threads;
using namespace smsc::util;

using namespace smsc::logger;

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

#define GET_BANNER_REQ_LEN 40       // общая длина фиксированных параметров в getBannerReq
#define ERR_INFO_LEN 16             // общая длинаstd::vector <char > buf; фиксированных параметров в errInfo

static int CommonTransactID = 0;    // общий счетчик транзакций
static Mutex CommTransId_mtx;       // мьютекс для доступа к счетчику

// коды команд 
enum 
  {
    CMD_BANNER_REQ = 1,    // запрос от клиента к серверу
    CMD_BANNER_RSP,        // ответ  от сервера
    CMD_ERR_INFO = 0x100   // пакет от клиента к серверу с информацией об ошибке
  };

static const unsigned CMD_HEADER = sizeof(uint32_t) + sizeof(uint32_t);
static const unsigned TRANSACTION_ID_LV_SIZE = sizeof(uint32_t) + sizeof(uint32_t);
static const unsigned BANNER_LEN_SIZE = sizeof(uint32_t);
static const unsigned BANNER_OFFSET_IN_PACKET = CMD_HEADER + TRANSACTION_ID_LV_SIZE;

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

class SimpleAdvertisingClient : public Advertising
{
public:
  SimpleAdvertisingClient(const std::string& host, int port, int timeout);

  virtual void init();

  virtual void reinit();

  // запрос банера
  virtual uint32_t getBanner( const std::string& abonent,
                              const std::string& serviceName,
                              uint32_t transportType, uint32_t charSet,
                              std::string &banner);

protected:
  Logger*     _logger; // указатель на общие журналы
  uint32_t    _timeout;
  std::string _host;
  in_port_t   _port;
  Socket _socket;
  bool _wasInited;

  uint32_t getBanner(BannerRequest& req);

  // проверка корректности асинхронного запроса
  void CheckBannerRequest(BannerRequest& banReq, int async = 0);

  //  заполнение буфера протокольной команды
  uint32_t PrepareCmnd(SerializationBuffer* rec,      // буфер
                       BannerRequest* par,            // параметры запроса банера
                       uint32_t cmndType,             // тип команды
                       uint32_t cmdInfo = 0);         // доп информация

  int sendRequestAndGetResponse(advertising_item* advItem, SerializationBuffer* req, uint32_t req_len);

  uint32_t readPacket(TmpBuf<char, MAX_PACKET_LEN>* buf);
  uint32_t readAdvert(advertising_item* advItem);
  int extractBanner(TmpBuf<char, MAX_PACKET_LEN>& incomingPacketBuf, std::string* banner);

  // запись в журнал сообщения об ошибке
  void WriteErrorToLog(char* where, int errCode);

  void readFromSocket(char *dataBuf, int bytesToRead, const std::string& where);
  void writeToSocket(const void* buf, int bufSize, const std::string& where);
};

}}

#endif

