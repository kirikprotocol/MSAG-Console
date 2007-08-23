#ifndef SCAG_ADVERT_ADVERTISING_CLIENT
#define SCAG_ADVERT_ADVERTISING_CLIENT

#include <string>

#include "core/synchronization/Mutex.hpp"

using namespace smsc::core::synchronization;

namespace scag {
namespace advert {

#define MAX_ASYNCS_COUNT 1024   // max count оf asynchronic requests in queue

// типы кодировок
enum 
  {
    UTF16BE = 1,
    GSMSMS,
    GSMUSSD
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

  BannerDispatcher() {};
  virtual ~BannerDispatcher() {};
};

struct BannerRequest
{
  std::string 	abonent;
  std::string 	serviceName;
  std::string 	banner;	  					 // filled by Advertising module
  int			transportType;
  int			maxLen;
  uint32_t    charSet;
  BannerDispatcher*   dispatcher;  // to call on banner comes or error occurs
    
  /**
   *  constructor
   *
   * @param   _abonent         abonent address in format .ton.npi.value   
   * @param   _serviceName     name of service      
   * @param   _transportType   type of transport (smpp/sms=1, smpp/ussd=2, wap/http=3, mms=4)          
   * @param   _charSet         charset of returning    
   * @param   _maxLen          maximum text len for text banner 
   * @param   _dispatcher      pointer to BannerDispatcher struct      
   *
   */
  BannerRequest(const std::string& _abonent, const std::string& _serviceName,
                int _transportType, uint32_t _charSet, int _maxLen=-1, 
                BannerDispatcher* _dispatcher = 0)
    : abonent(_abonent), serviceName(_serviceName), banner(""), 
      transportType(_transportType), charSet(_charSet), 
      maxLen(_maxLen), dispatcher(_dispatcher), id(getNextId()) {};
	  
  /**
   *  copy constructor
   *
   * @param	req         source
   */
  BannerRequest(const BannerRequest& req)
    : abonent(req.abonent), serviceName(req.serviceName), 
      banner(req.banner), transportType(req.transportType), 
      charSet(req.charSet), maxLen(req.maxLen), 
      dispatcher(req.dispatcher), id(req.id) {};
	  
  /**
   *  operator = 
   *
   * @param	req         source
   *
   * @return 	BannerRequest& 	
   */
  BannerRequest& operator=(const BannerRequest& req) 
  {
    abonent = req.abonent; serviceName = req.serviceName; 
    banner = req.banner; transportType = req.transportType; 
    charSet = req.charSet; maxLen = req.maxLen;
    dispatcher = req.dispatcher; id = req.id;
    return (*this);   
  };
    
  /**
   *  return value of Id 
   *
   * @return 	uint64_t 	
   */
  inline uint64_t getId() {
    return id;
  };
	  
protected:

  uint64_t    id; // increment on creation
    
  static uint64_t lastId;
  static Mutex	lastIdMutex;
    
  /**
   *  return the lastest value of BannerRequests Id 
   *
   * @return 	uint64_t 	
   */
  static uint64_t getNextId() {
    MutexGuard mg(lastIdMutex);
    return ++lastId;
  };
};

struct Advertising 
{
  /**
   *  Initing of concret instance and setting to one socket params
   *
   * @param	host    		IP addres of server
   * @param	port            port number
   * @param	timeout         time (msec) waiting of server's response
   * @param	maxcount        maximum count of requests (in queue) to server
   *
   * @return 	void 		no returns
   */
  static void Init(const std::string& host, int port, int timeout=0, int maxcount = MAX_ASYNCS_COUNT);
    
  /**
   *  return pointer of existing instance 
   *
   * @return 	Advertising& 	
   */
  static  Advertising& Instance();
    
  /**
   * Returns 0 - if OK  or  error code (see AdvertErrors.h)
   *
   * @param	abonent		abonent address in format .ton.npi.value
   * @param	serviceName	name of service
   * @param	transportType	type of transport (smpp/sms=1, smpp/ussd=2, wap/http=3, mms=4)
   * @param	charSet		
   * @param	banner		text banner to show, empty if no banner
   * @param	maxLen		maximum text len for text banner
   *
   * @return 	int 		0 (if success) or error code
   */
  virtual uint32_t getBanner(const std::string& abonent, 
                             const std::string& serviceName,     
                             uint32_t transportType, uint32_t charSet, 
                             std::string &banner, uint32_t maxLen=(uint32_t)-1) = 0;
	 
  /**
   * Returns 0 - if OK  or  error code (see AdvertErrors.h)
   *
   * @param	req 		structure with request's params
   *
   * @return 	int 		0 (if success) or error code
   */
  virtual uint32_t getBanner(BannerRequest& req) = 0;
    
  /**
   * Initiate request for banner
   *
   * @param	req 		structure with request's params
   */
  virtual void requestBanner(const BannerRequest& req) = 0;
    
  /**
   * Break work of implementation's threads
   */
  virtual void Stop() {};
			
protected:
  Advertising() {}
  virtual ~Advertising() {};
};

} // advert
} // scag

#endif //SCAG_ADVERT_ADVERTISING_CLIENT
