#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMEREGISTRY_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_SMEREGISTRY_HPP__

# include <string>
# include <map>
# include <list>

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

std::string extractSmePeerAddr(const std::string& link_id_to_sme);

struct SmeInfo {
  SmeInfo(const std::string& system_id,
          const io_subsystem::LinkId& link_id_to_sme,
          const io_subsystem::LinkId& dst_link_set_id,
          unsigned io_proc_id,
          unsigned io_proc_mgr_id,
          BindRequest* bind_request)
  : systemId(system_id), linkIdToSme(link_id_to_sme), dstLinkSetId(dst_link_set_id),
    ioProcId(io_proc_id), ioProcMgrId(io_proc_mgr_id),
    bindRequest(bind_request)
  {}
  ~SmeInfo() { delete bindRequest; }
  std::string systemId;
  io_subsystem::LinkId linkIdToSme, dstLinkSetId;
  unsigned ioProcId, ioProcMgrId;
  BindRequest* bindRequest;
};

typedef smsc::core::buffers::RefPtr<SmeInfo, smsc::core::synchronization::Mutex> SmeInfoRef;

struct SmeTrace {
  SmeTrace(const std::string& bind_mode,
           const std::string& peer_addr,
           const std::string& system_id,
           const io_subsystem::LinkId& dst_linkset_id,
           unsigned io_proc_mgr_id, unsigned io_proc_id)
  : bindMode(bind_mode), peerAddr(peer_addr), systemId(system_id),
    dstLinksetId(dst_linkset_id), ioProcMgrId(io_proc_mgr_id), ioProcId(io_proc_id)
  {}
  std::string bindMode;
  std::string peerAddr;
  std::string systemId;
  io_subsystem::LinkId dstLinksetId;
  unsigned ioProcMgrId, ioProcId;
};

typedef std::list<SmeTrace> active_sme_list_t;

class SmeRegistry : public utilx::Singleton<SmeRegistry> {
public:
  SmeRegistry()
  : _logger(smsc::logger::Logger::getInstance("smpp"))
  {}

  void registerSme(SmeInfo* sme_info);

  SmeInfoRef getSmeInfo(const std::string& system_id);
  SmeInfoRef getSmeInfo(const io_subsystem::LinkId& link_id_to_sme);

  void deleteSmeInfo(const io_subsystem::LinkId& link_id_to_sme);
  void getActiveSmeList(active_sme_list_t* active_sme_list);

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
