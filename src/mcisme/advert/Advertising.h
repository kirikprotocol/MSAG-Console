#ifndef MCISME_ADVERT_ADVERTISING_CLIENT
#define MCISME_ADVERT_ADVERTISING_CLIENT

#include <string>

#include <core/synchronization/Mutex.hpp>
#include <logger/Logger.h>

using namespace smsc::core::synchronization;

namespace smsc {
namespace mcisme {

#define MAX_ASYNCS_COUNT 1024   // max count оf asynchronic requests in queue

// типы кодировок
enum 
  {
    UTF16BE = 1,
    GSMSMS,
    GSMUSSD,
    ASCII_TRANSLIT
  };

enum
  {
    SMPP_SMS = 1,
    SMPP_USSD,
    WAP_HTTP,
    MMS
  };

struct BannerRequest;
struct BannerDispatcher
{
  // On banner received
  virtual void  processBanner(const BannerRequest& req) = 0;

  // On error occurred
  virtual void processError(const BannerRequest& req, uint32_t status) = 0;

protected:

  BannerDispatcher() {}
  virtual ~BannerDispatcher() {}
};

struct BannerRequest
{
  std::string abonent;
  std::string serviceName;
  std::string banner; // filled by Advertising module
  int         transportType;
  uint32_t    charSet;
  uint32_t bannerId, ownerId, rotatorId;
  /**
   *  constructor
   *
   * @param   _abonent         abonent address in format .ton.npi.value
   * @param   _serviceName     name of service
   * @param   _transportType   type of transport (smpp/sms=1, smpp/ussd=2, wap/http=3, mms=4)
   * @param   _charSet         charset of returning
   * @param   _dispatcher      pointer to BannerDispatcher struct
   *
   */
  BannerRequest(const std::string& _abonent, const std::string& _serviceName,
                int _transportType, uint32_t _charSet)
    : abonent(_abonent), serviceName(_serviceName), banner(""),
      transportType(_transportType), charSet(_charSet),
      bannerId(-1), ownerId(0), rotatorId(0),
      id(getNextId()) {}

  /**
   *  return value of Id
   */
  inline uint32_t getId() {
    return id;
  }
protected:

  uint32_t    id; // increment on creation

  static uint32_t lastId;
  static Mutex	lastIdMutex;

  /**
   *  return the lastest value of BannerRequests Id
   *
   * @return uint64_t
   */
  static uint32_t getNextId() {
    MutexGuard mg(lastIdMutex);
    return ++lastId;
  }
};

class Advertising {
public:
  virtual void init(int connectTimeout=0) = 0;
  virtual void reinit(int connectTimeout=0) = 0;
  /**
   * Returns 0 - if OK  or  error code (see AdvertErrors.h)
   *
   * @param abonent         abonent address in format .ton.npi.value
   * @param serviceName     name of service
   * @param transportType   type of transport (smpp/sms=1, smpp/ussd=2, wap/http=3, mms=4)
   * @param charSet
   * @param banner          text banner to show, empty if no banner
   *
   * @return int            0 (if success) or error code
   */
  virtual uint32_t getBanner(const std::string& abonent,
                             const std::string& serviceName,
                             uint32_t transportType, uint32_t charSet,
                             std::string &banner) = 0;

  virtual ~Advertising() {}

protected:
  Advertising()
    : _logger(logger::Logger::getInstance("scag.advert.Advertising")) {}

  logger::Logger* _logger;
};

}}

#endif
