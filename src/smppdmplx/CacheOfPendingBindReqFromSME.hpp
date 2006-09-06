#ifndef __SMPPDMPLX_CACHEOFPENDINGBINDREQFROMSME_HPP__
# define __SMPPDMPLX_CACHEOFPENDINGBINDREQFROMSME_HPP__ 1

# include <string>
# include <list>
# include <map>
# include <utility>
# include <memory>
# include <util/Singleton.hpp>

# include "SMPPSession.hpp"
# include "SMPP_BindRequest.hpp"
# include "SMPP_BindResponse.hpp"

namespace smpp_dmplx {

class CacheOfPendingBindReqFromSME : public smsc::util::Singleton<CacheOfPendingBindReqFromSME> {
public:
  void putPendingReqToCache(SMPPSession& smppSessionToSME, SMPP_BindRequest* bindRequest);
  void commitPendingReqInCache(const std::string& systemId, const SMPP_BindResponse& bindResp);
private:
  //CacheOfPendingBindReqFromSME() {}
  typedef std::pair<SMPPSession, SMPP_BindRequest*>  cached_request_t;
  typedef std::list<cached_request_t> list_of_cached_request_t;
  typedef std::map<std::string, list_of_cached_request_t*> sysid_to_cache_of_requests_map_t;

  sysid_to_cache_of_requests_map_t map_sysIdToRequestCache;
};

}

#endif
