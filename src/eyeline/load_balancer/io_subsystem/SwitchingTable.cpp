#include <utility>

#include "core/synchronization/MutexGuard.hpp"
#include "SwitchingTable.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

void
SwitchingTable::insertSwitching(const LinkId& sme_link_id,
                                const LinkId& smsc_linkset_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_switchTableLock);
  smsc_log_debug(_logger, "SwitchingTable::insertSwitching::: insert route in_link='%s'-->out_linkset='%s'",
                 sme_link_id.toString().c_str(), smsc_linkset_id.toString().c_str());
  std::pair<switch_table_t::iterator, bool> ins_res
    = _smeToSmcLinkset.insert(std::make_pair(sme_link_id, smsc_linkset_id));
  if ( ins_res.second ) {
    smsc_log_debug(_logger, "SwitchingTable::insertSwitching::: route in_link='%s'-->out_linkset='%s' has been inserted",
                     sme_link_id.toString().c_str(), smsc_linkset_id.toString().c_str());
    ins_res = _smscLinksetToSme.insert(std::make_pair(smsc_linkset_id, sme_link_id));
    if ( !ins_res.second )
      throw smsc::util::Exception("SwitchingTable::insertSwitching::: can't insert mapping smsc_linkset ==> sme_link");
    smsc_log_debug(_logger, "SwitchingTable::insertSwitching::: route out_linkset='%s'-->in_link='%s' has been inserted",
                   smsc_linkset_id.toString().c_str(), sme_link_id.toString().c_str());
  } else
    throw smsc::util::Exception("SwitchingTable::insertSwitching::: can't insert mapping sme_link ==> smsc_linkset");
}

bool
SwitchingTable::getSwitching(const LinkId& src_link_id,
                             LinkId* dst_link_id,
                             bool* is_src_link_incoming) const
{
  bool isSrcLinkIncoming;
  {
    smsc_log_debug(_logger, "SwitchingTable::getSwitching::: try get route for source link with id='%s'",
                   src_link_id.toString().c_str());

    smsc::core::synchronization::MutexGuard synchronize(_switchTableLock);
    switch_table_t::const_iterator iter = _smeToSmcLinkset.find(src_link_id);
    if ( iter != _smeToSmcLinkset.end() )
      isSrcLinkIncoming = true;
    else {
      smsc_log_debug(_logger, "SwitchingTable::getSwitching::: route from sme (linkid=%s) not found, try find route from smsc",
                     src_link_id.toString().c_str());
      iter = _smscLinksetToSme.find(src_link_id);
      if ( iter == _smscLinksetToSme.end() )
        return false;

      isSrcLinkIncoming = false;
    }

    *dst_link_id = iter->second;
  }
  if ( isSrcLinkIncoming )
    smsc_log_debug(_logger, "SwitchingTable::getSwitching::: got linkset with id='%s' for link from sme with id='%s'",
                   dst_link_id->toString().c_str(), src_link_id.toString().c_str());
  else
    smsc_log_debug(_logger, "SwitchingTable::getSwitching::: got link to sme with id='%s' for linkset with id='%s'",
                   dst_link_id->toString().c_str(), src_link_id.toString().c_str());

  if  ( is_src_link_incoming )
    *is_src_link_incoming = isSrcLinkIncoming;

  return true;
}

bool
SwitchingTable::removeSwitching(const LinkId& sme_link_id,
                                LinkId* smsc_linkset_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_switchTableLock);
  switch_table_t::iterator iter = _smeToSmcLinkset.find(sme_link_id);

  smsc_log_debug(_logger, "SwitchingTable::removeSwitching::: try remove route for sme link with id='%s'",
                 sme_link_id.toString().c_str());

  if ( iter == _smeToSmcLinkset.end() )
    return false;

  if ( smsc_linkset_id )
    *smsc_linkset_id = iter->second;

  _smeToSmcLinkset.erase(iter);
  return true;
}

void
SwitchingTable::setSpecificSwitching(const LinkId& linkset_id,
                                     const LinkId& specific_link_id,
                                     unsigned color)
{
  smsc::core::synchronization::MutexGuard synchronize(_specificSwitchTableLock);
  std::pair<specific_switch_table_t::iterator, bool> ins_res =
    _specificSwitchTable.insert(std::make_pair(specific_switch_key(linkset_id, color),
                                               specific_link_id));
  if ( !ins_res.second )
    throw smsc::util::Exception("SwitchingTable::insertSwitching::: can't insert mapping smsc_linkset, color ==> sme_link, mapping already exists");
}

LinkId
SwitchingTable::getSpecificSwitching(const LinkId& linkset_id,
                                     unsigned color) const
{
  smsc::core::synchronization::MutexGuard synchronize(_specificSwitchTableLock);
  specific_switch_table_t::const_iterator iter =
    _specificSwitchTable.find(specific_switch_key(linkset_id, color));

  if ( iter == _specificSwitchTable.end() )
    return LinkId("");

  return iter->second;
}

void
SwitchingTable::removeSpecificSwitching(const LinkId& linkset_id,
                                        unsigned color)
{
  smsc::core::synchronization::MutexGuard synchronize(_specificSwitchTableLock);
  _specificSwitchTable.erase(specific_switch_key(linkset_id, color));
}

}}}
