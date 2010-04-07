#ifndef __EYELINE_SS7NA_SUAGW_SCCPSAP_LINKSETINFOREGISTRY_HPP__
# define __EYELINE_SS7NA_SUAGW_SCCPSAP_LINKSETINFOREGISTRY_HPP__

# include <set>
# include <map>
# include <string>

# include "logger/Logger.h"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {

class LinkSetInfoRegistry : public utilx::Singleton<LinkSetInfoRegistry> {
public:
  void addAssociation(const common::LinkId& linkset_id, const std::string& app_id);

  // return true if 'appId --> linkSet' association was found, else return false.
  bool getLinkSet(const std::string& app_id, common::LinkId* linkset_id) const;

  void removeAssociation(const common::LinkId& linkset_id, const std::string& app_id);

  std::set<common::LinkId> getLinkSetIds() const;

  bool getAppId(const common::LinkId& linkset_id, std::string* app_id);
private:
  LinkSetInfoRegistry()
  : _logger(smsc::logger::Logger::getInstance("sccp_sap"))
  {}
  friend class utilx::Singleton<LinkSetInfoRegistry>;

  smsc::logger::Logger* _logger;
  typedef std::map<common::LinkId, std::set<std::string>* /*appIds*/> registry_t;
  registry_t _registry;
  mutable smsc::core::synchronization::Mutex _lock;
};

}}}}

#endif
