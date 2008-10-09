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

#define GET_BANNER_REQ_LEN 40       // ����� ����� ������������� ���������� � getBannerReq
#define ERR_INFO_LEN 16             // ����� �����std::vector <char > buf; ������������� ���������� � errInfo

static int CommonTransactID = 0;    // ����� ������� ����������
static Mutex CommTransId_mtx;       // ������� ��� ������� � ��������

// ���� ������ 
enum 
  {
    CMD_BANNER_REQ = 1,    // ������ �� ������� � �������
    CMD_BANNER_RSP,        // �����  �� �������
    CMD_ERR_INFO = 0x100   // ����� �� ������� � ������� � ����������� �� ������
  };

static const unsigned CMD_HEADER = sizeof(uint32_t) + sizeof(uint32_t);
static const unsigned TRANSACTION_ID_LV_SIZE = sizeof(uint32_t) + sizeof(uint32_t);
static const unsigned BANNER_LEN_SIZE = sizeof(uint32_t);
static const unsigned BANNER_OFFSET_IN_PACKET = CMD_HEADER + TRANSACTION_ID_LV_SIZE;

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

class SimpleAdvertisingClient : public Advertising
{
public:
  SimpleAdvertisingClient(const std::string& host, int port, int timeout);

  virtual void init();

  virtual void reinit();

  // ������ ������
  virtual uint32_t getBanner( const std::string& abonent,
                              const std::string& serviceName,
                              uint32_t transportType, uint32_t charSet,
                              std::string &banner);

protected:
  Logger*     _logger; // ��������� �� ����� �������
  uint32_t    _timeout;
  std::string _host;
  in_port_t   _port;
  Socket _socket;
  bool _wasInited;

  uint32_t getBanner(BannerRequest& req);

  // �������� ������������ ������������ �������
  void CheckBannerRequest(BannerRequest& banReq, int async = 0);

  //  ���������� ������ ������������ �������
  uint32_t PrepareCmnd(SerializationBuffer* rec,      // �����
                       BannerRequest* par,            // ��������� ������� ������
                       uint32_t cmndType,             // ��� �������
                       uint32_t cmdInfo = 0);         // ��� ����������

  int sendRequestAndGetResponse(advertising_item* advItem, SerializationBuffer* req, uint32_t req_len);

  uint32_t readPacket(TmpBuf<char, MAX_PACKET_LEN>* buf);
  uint32_t readAdvert(advertising_item* advItem);
  int extractBanner(TmpBuf<char, MAX_PACKET_LEN>& incomingPacketBuf, std::string* banner);

  // ������ � ������ ��������� �� ������
  void WriteErrorToLog(char* where, int errCode);

  void readFromSocket(char *dataBuf, int bytesToRead, const std::string& where);
  void writeToSocket(const void* buf, int bufSize, const std::string& where);
};

}}

#endif

