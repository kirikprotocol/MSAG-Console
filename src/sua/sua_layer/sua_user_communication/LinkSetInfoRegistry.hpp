#ifndef __SUA_SUALAYER_SUAUSERCOMMUNICATION_LINKSETINFOREGISTRY_HPP__
# define __SUA_SUALAYER_SUAUSERCOMMUNICATION_LINKSETINFOREGISTRY_HPP__ 1

# include <set>
# include <map>
# include <string>
# include <logger/Logger.h>
# include <sua/utilx/Singleton.hpp>
# include <sua/communication/LinkId.hpp>
# include <core/synchronization/Mutex.hpp>

namespace sua_user_communication {

class LinkSetInfoRegistry : public utilx::Singleton<LinkSetInfoRegistry> {
public:
  LinkSetInfoRegistry();

  void addAssociation(const communication::LinkId& linkSetId, const std::string& appId);

  // return true if 'appId --> linkSet' association was found, else return false.
  bool getLinkSet(const std::string& appId, communication::LinkId* linkSetId) const;

  void removeAssociation(const communication::LinkId& linkSetId, const std::string& appId);

  std::set<communication::LinkId> getLinkSetIds() const;

  bool getAppId(const communication::LinkId& linkSetId, std::string* appId);
private:
  smsc::logger::Logger* _logger;
  typedef std::map<communication::LinkId, std::set<std::string>* /*appIds*/> registry_t;
  registry_t _registry;
  mutable smsc::core::synchronization::Mutex _lock;
};

}

#endif
