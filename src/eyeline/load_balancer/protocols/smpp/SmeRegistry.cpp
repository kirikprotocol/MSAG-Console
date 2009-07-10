#include <utility>

#include "SmeRegistry.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "core/synchronization/MutexGuard.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
SmeRegistry::registerSme(SmeInfo* sme_info)
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);
  if ( _activeSmes.find(sme_info->systemId) == _activeSmes.end() ) {
    std::pair<active_smes_t::iterator, bool> ins_res =
      _activeSmes.insert(std::make_pair(sme_info->systemId, SmeInfoRef(sme_info)));
    _activeSmeRefs.insert(std::make_pair(sme_info->linkIdToSme, ins_res.first));
  } else {
    smsc_log_error(_logger, "SmeRegistry::registerSme::: sme with id='%s' already registered",
                   sme_info->systemId.c_str());
    throw utilx::DuplicatedRegistryKeyException("SmeRegistry::registerSme::: duplicated sme with id='%s'",
                                                sme_info->systemId.c_str());
  }
}

SmeInfoRef
SmeRegistry::getSmeInfo(const std::string& system_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);
  active_smes_t::iterator iter = _activeSmes.find(system_id);
  if ( iter == _activeSmes.end() )
    throw utilx::RegistryKeyNotFound("SmeRegistry::getSmeInfo::: there isn't sme for sme_id='%s'",
                                     system_id.c_str());
  return iter->second;
}

SmeInfoRef
SmeRegistry::getSmeInfo(const io_subsystem::LinkId& link_id_to_sme)
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);
  active_sme_refs_t::iterator iter = _activeSmeRefs.find(link_id_to_sme);
  if ( iter == _activeSmes.end() )
    throw utilx::RegistryKeyNotFound("SmeRegistry::getSmeInfo::: there isn't sme for linkid='%s'",
                                     link_id_to_sme.toString().c_str());
  return iter->second->second;
}

void
SmeRegistry::deleteSmeInfo(const io_subsystem::LinkId& link_id_to_sme)
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);
  active_sme_refs_t::iterator iter = _activeSmeRefs.find(link_id_to_sme);
  if ( iter == _activeSmeRefs.end() )
    return;

  smsc_log_debug(_logger, "SmeRegistry::deleteSmeInfo::: delete sme info for link with id='%s'",
                 link_id_to_sme.toString().c_str());
  _activeSmes.erase(iter->second);
  _activeSmeRefs.erase(iter);
}

}}}}
