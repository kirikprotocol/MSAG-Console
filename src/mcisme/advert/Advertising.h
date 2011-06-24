#ifndef MCISME_ADVERT_ADVERTISING_CLIENT
#define MCISME_ADVERT_ADVERTISING_CLIENT

#include <string>

#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"
#include "mcisme/Messages.h"

namespace smsc {
namespace mcisme {

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
  uint32_t    bannerId, ownerId, rotatorId;
  /*
     fd value that used to send banner request.
     this value will be also used to send rollback request
     if request's timeout is expired
  */
  int         fd;
  MCEventOut* mcEventOut;

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
                int transport_type, uint32_t char_set, uint32_t max_banner_size,
                MCEventOut* mc_event_out)
    : abonent(an_abonent), serviceName(service_name), banner(""),
      transportType(transport_type), maxBannerSize(max_banner_size), charSet(char_set),
      bannerId(-1), ownerId(0), rotatorId(0), fd(-1), mcEventOut(mc_event_out), id(getNextId())
  {}

  // banner rollback request
  BannerRequest(uint32_t aTransactionId, uint32_t aBannerId, uint32_t anOwnerId, uint32_t aRotatorId)
    : id(aTransactionId), bannerId(aBannerId),
      ownerId(anOwnerId), rotatorId(aRotatorId), fd(-1), mcEventOut(NULL)
  {}

  ~BannerRequest() {
    delete mcEventOut;
  }

  void resetMCEventOut() { mcEventOut = NULL; }

  /**
   *  return value of Id
   */
  inline uint32_t getId() const {
    return id;
  }
protected:

  uint32_t    id; // increment on creation

  static uint32_t lastId;
  static core::synchronization::Mutex lastIdMutex;

  /**
   *  return the lastest value of BannerRequests Id
   *
   * @return uint64_t
   */
  static uint32_t getNextId() {
    core::synchronization::MutexGuard mg(lastIdMutex);
    return ++lastId;
  }
};

struct BannerResponseTrace {
  BannerResponseTrace()
    : transactionId(0), bannerId(0), ownerId(0), rotatorId(0) {}

  uint32_t transactionId, bannerId, ownerId, rotatorId;

  bool operator != (const BannerResponseTrace& rhs) {
    if ( transactionId != rhs.transactionId ||
         bannerId != rhs.bannerId ||
         ownerId != rhs.ownerId ||
         rotatorId != rhs.rotatorId )
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
   * @param abonent          abonent address in format .ton.npi.value
   * @param service_name     name of service
   * @param transport_type   type of transport (smpp/sms=1, smpp/ussd=2, wap/http=3, mms=4)
   * @param char_set
   * @param max_banner_size  max banner size that could be formed
   * @param mc_event_out     event that should be sent
   * @return int            0 (if success) or error code
   */
  virtual uint32_t sendBannerRequest(const std::string& abonent,
                                     const std::string& service_name,
                                     uint32_t transport_type, uint32_t char_set,
                                     uint32_t max_banner_size, MCEventOut* mc_event_out) = 0;

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
