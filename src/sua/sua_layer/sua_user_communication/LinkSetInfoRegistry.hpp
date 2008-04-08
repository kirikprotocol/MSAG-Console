#ifndef __SUA_SUALAYER_SUAUSERCOMMUNICATION_LINKSETINFOREGISTRY_HPP__
# define __SUA_SUALAYER_SUAUSERCOMMUNICATION_LINKSETINFOREGISTRY_HPP__ 1

# include <set>
# include <map>
# include <string>
# include <logger/Logger.h>
# include <sua/utilx/Singleton.hpp>
# include <sua/communication/LinkId.hpp>

namespace sua_user_communication {

class LinkSetInfoRegistry : public utilx::Singleton<LinkSetInfoRegistry> {
public:
  LinkSetInfoRegistry();

  void addAssociation(const communication::LinkId& linkSetId, const std::string& appId);
  
  const communication::LinkId& getLinkSet(const std::string& appId);
  
  void removeAssociation(const communication::LinkId& linkSetId, const std::string& appId);

private:
  smsc::logger::Logger* _logger;
  typedef std::map<communication::LinkId, std::set<std::string>* /*appIds*/> registry_t;
  registry_t _registry;
};

}

#endif
