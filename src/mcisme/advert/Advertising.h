#ifndef MCISME_ADVERT_ADVERTISING_CLIENT
#define MCISME_ADVERT_ADVERTISING_CLIENT

#include <string>

#include <core/synchronization/Mutex.hpp>
#include <logger/Logger.h>

namespace smsc {
namespace mcisme {

using namespace smsc::core::synchronization;

#define MAX_ASYNCS_COUNT 1024   // max count �f asynchronic requests in queue

// ���� ���������
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

struct BannerRequest
{
  std::string abonent;
  std::string serviceName;
  std::string banner; // filled by Advertising module
  int         transportType;
  uint32_t    maxBannerSize;
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
  BannerRequest(const std::string& an_abonent, const std::string& service_name,
                int transport_type, uint32_t char_set, uint32_t max_banner_size)
    : abonent(an_abonent), serviceName(service_name), banner(""),
      transportType(transport_type), maxBannerSize(max_banner_size), charSet(char_set),
      bannerId(-1), ownerId(0), rotatorId(0),
      id(getNextId()) {}

  BannerRequest(uint32_t aTransactionId, uint32_t aBannerId, uint32_t anOwnerId, uint32_t aRotatorId)
    : id(aTransactionId), bannerId(aBannerId),
      ownerId(anOwnerId), rotatorId(aRotatorId) {}
  /**
   *  return value of Id
   */
  inline uint32_t getId() const {
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

struct BannerResponseTrace {
  BannerResponseTrace()
    : transactionId(0), bannerId(0), ownerId(0), rotatorId(0), bannerIdIsNotUsed(false) {}

  uint32_t transactionId, bannerId, ownerId, rotatorId;
  bool bannerIdIsNotUsed;

  bool operator != (const BannerResponseTrace& rhs) {
    if ( transactionId != rhs.transactionId ||
         bannerId != rhs.bannerId ||
         ownerId != rhs.ownerId ||
         rotatorId != rhs.rotatorId ||
         bannerIdIsNotUsed != rhs.bannerIdIsNotUsed)
      return true;
    else
      return false;
  }
};

class Advertising {
public:
  virtual void init(int connectTimeout=0) = 0;
  virtual bool reinit(int connectTimeout=0) = 0;
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
                             const std::string& service_name,
                             uint32_t transport_type, uint32_t char_set,
                             std::string* banner,
                             BannerResponseTrace* banner_resp_trace,
                             size_t max_banner_size) = 0;

  virtual void rollbackBanner(uint32_t transactionId,
                              uint32_t bannerId,
                              uint32_t ownerId,
                              uint32_t rotatorId) = 0;

  virtual ~Advertising() {}

protected:
  Advertising()
    : _logger(logger::Logger::getInstance("advert")) {}

  logger::Logger* _logger;
};

}}

#endif
