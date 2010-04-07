#ifndef __EYELINE_SS7NA_M3UAGW_COMMON_SCCPSAP_ACTIVEAPPSREGISTRY_HPP__
# define __EYELINE_SS7NA_M3UAGW_COMMON_SCCPSAP_ACTIVEAPPSREGISTRY_HPP__

# include <map>
# include <list>

# include <string.h>
# include <sys/types.h>

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

class ActiveAppsRegistry : public utilx::Singleton<ActiveAppsRegistry> {
public:
  // get first active application servicing subsystem with specified ssn
  LinkId getLink(uint8_t ssn) const;

  // get link for active application with specified app_id value and
  // that service subsystem with specified ssn value
  LinkId getLink(const std::string& app_id, uint8_t ssn) const;

  // get link for active application with specified app_id value
  LinkId getLink(const std::string& app_id) const;

  void insert(const uint8_t* ssn_begin, const uint8_t* ssn_end,
              const LinkId& link_id_to_application, const std::string& app_id);
  void remove(const LinkId& link_id_to_application);

  void getAllActiveAppLinks(std::list<LinkId>* linkid_list) const;

private:
  ActiveAppsRegistry()
  : _logger(smsc::logger::Logger::getInstance("sccp"))
  {
    memset(_activeSsns, 0, MAX_COUNT_OF_SSN*sizeof(connected_apps*));
  }

  friend class utilx::Singleton<ActiveAppsRegistry>;

  smsc::logger::Logger* _logger;
  struct connected_apps {
    std::list<LinkId> activeApplications;
  };
  enum { MAX_COUNT_OF_SSN = 256 };
  connected_apps* _activeSsns[MAX_COUNT_OF_SSN];

  struct app_info {
    app_info(const LinkId& link_id,
             const uint8_t* ssn_begin, const uint8_t* ssn_end)
    : linkId(link_id), maxSsnNum(0)
    {
      memset(serviceableSsns, 0, MAX_COUNT_OF_SSN);
      while(ssn_begin != ssn_end) {
        uint8_t ssnValue = *ssn_begin++;
        serviceableSsns[ssnValue] = 1;
        maxSsnNum = std::max(maxSsnNum, ssnValue);
      }
    }
    LinkId linkId;
    uint8_t maxSsnNum;
    uint8_t serviceableSsns[MAX_COUNT_OF_SSN];
  };
  typedef std::map<std::string /*appId*/, app_info> active_apps_t;
  active_apps_t _activeApps;

  typedef std::map<LinkId, active_apps_t::iterator> active_links_t;
  active_links_t _activeLinks;

  mutable smsc::core::synchronization::Mutex _lock;
};

}}}}

#endif
