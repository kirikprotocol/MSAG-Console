#ifndef __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_LINKSETINFOREGISTRY_HPP__
# define __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_LINKSETINFOREGISTRY_HPP__

# include <set>
# include <map>
# include <string>
# include <logger/Logger.h>
# include <core/synchronization/Mutex.hpp>
# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/sua/communication/LinkId.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
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

}}}}

#endif
