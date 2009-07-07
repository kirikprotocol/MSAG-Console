#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMEREGISTRY_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMEREGISTRY_HPP__

# include <string>
# include <map>

# include "logger/Logger.h"
# include "core/buffers/RefPtr.hpp"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/protocols/smpp/BindRequest.hpp"
# include "eyeline/utilx/Singleton.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

struct SmeInfo {
  SmeInfo(const std::string& system_id,
          const io_subsystem::LinkId& link_id_to_sme,
          BindRequest* bind_request)
  : systemId(system_id), linkIdToSme(link_id_to_sme),
    bindRequest(bind_request)
  {}
  ~SmeInfo() { delete bindRequest; }
  std::string systemId;
  io_subsystem::LinkId linkIdToSme;
  BindRequest* bindRequest;
};

typedef smsc::core::buffers::RefPtr<SmeInfo, smsc::core::synchronization::Mutex> SmeInfoRef;

class SmeRegistry : public utilx::Singleton<SmeRegistry> {
public:
  SmeRegistry()
  : _logger(smsc::logger::Logger::getInstance("smpp"))
  {}

  void registerSme(SmeInfo* sme_info);

  SmeInfoRef getSmeInfo(const std::string& system_id);
  SmeInfoRef getSmeInfo(const io_subsystem::LinkId& link_id_to_sme);

  void deleteSmeInfo(const io_subsystem::LinkId& link_id_to_sme);
private:
  smsc::logger::Logger* _logger;

  smsc::core::synchronization::Mutex _lock;
  typedef std::string system_id_t;
  typedef std::map<system_id_t, SmeInfoRef> active_smes_t;
  active_smes_t _activeSmes;
  typedef std::map<io_subsystem::LinkId, active_smes_t::iterator> active_sme_refs_t;
  active_sme_refs_t _activeSmeRefs;
};

}}}}

#endif
