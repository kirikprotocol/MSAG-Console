#include <utility>
#include "GTTranslationTable.hpp"
#include "eyeline/ss7na/common/types.hpp"
#include "eyeline/ss7na/common/Exception.hpp"
#include "eyeline/ss7na/m3ua_gw/types.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace router {

GTTranslationTable::GTTranslationTable()
  : _logger(smsc::logger::Logger::getInstance("sccp_rout"))
{}

GTTranslationResult
GTTranslationTable::translate(const eyeline::sccp::SCCPAddress& address)
{
  const char* gtValue = address.getGT().getSignals();

  if ( address.getIndicator().hasSSN )
    smsc_log_debug(_logger, "GTTranslationTable::translate::: try find dpc for routing key=[gt=%s,ssn=%d]",
                   gtValue, address.getSSN());
  else
    smsc_log_debug(_logger, "GTTranslationTable::translate::: try find dpc for routing key=[gt=%s]",
                   gtValue);

  RouteTableEntry *routeEntry=NULL;
  bool routeWasFound = true, searchResult;

  smsc_log_debug(_logger, "GTTranslationTable::translate::: call _routingTable.FindPrefix(%s)", gtValue);
  {
    smsc::core::synchronization::MutexGuard synchronize(_routingTableLock);
    searchResult = _routingTable.FindPrefix(gtValue, routeEntry);
  }
  std::string routeId;
  if ( searchResult ) {
    smsc_log_debug(_logger, "GTTranslationTable::translate::: found entry for gt=[%s]", gtValue);
    if ( address.getIndicator().hasSSN &&
        translationResultIncludesSsn(routeEntry) ) {
      routeWasFound = findRouteBySSN(&routeId, routeEntry, address.getSSN());
      if ( routeWasFound ) {
        smsc_log_info(_logger, "GTTranslationTable::translate::: found route with routeId=[%s] for routing key=[gt=%s,ssn=%d]",
                      routeId.c_str(), gtValue, address.getSSN());
        return GTTranslationResult(routeId, false);
      }
    } else {
      smsc_log_info(_logger, "GTTranslationTable::translate::: found route with routeId=[%s] for routing key=[gt=%s]",
                    routeEntry->routeId.c_str(), gtValue);
      return GTTranslationResult(routeEntry->routeId, routeEntry->routeToMTP3);
    }

    if ( address.getIndicator().hasSSN )
      smsc_log_info(_logger, "GTTranslationTable::translate::: route not found for routing key=[gt=%s,ssn=%d]", gtValue, address.getSSN());
    else
      smsc_log_info(_logger, "GTTranslationTable::translate::: route not found for routing key=[gt=%s]", gtValue);
    throw common::TranslationFailure(common::NO_ADDR_TRANSLATION_FOR_SPECIFIC_ADDR, "GTTranslationTable::translate::: route not found");
  } else
    throw common::TranslationFailure(common::NO_ADDR_TRANSLATION_FOR_SPECIFIC_ADDR, "GTTranslationTable::translate::: route not found");

  return GTTranslationResult("", false); //to avoid compiler warning
}

bool
GTTranslationTable::translationResultIncludesSsn(const RouteTableEntry* route_entry)
{
  smsc::core::synchronization::MutexGuard synchronize(route_entry->routingEntryLock);
  return !route_entry->routeBySsn.empty();
}

bool
GTTranslationTable::findRouteBySSN(std::string* route_id,
                                   const RouteTableEntry* route_entry,
                                   uint8_t destination_ssn)
{
  {
    smsc::core::synchronization::MutexGuard synchronize(route_entry->routingEntryLock);
    smsc_log_debug(_logger, "GTTranslationTable::findRouteBySSN::: try match against ssn=%d", destination_ssn);
    std::map<uint8_t, std::string>::const_iterator iter = route_entry->routeBySsn.find(destination_ssn);
    if ( iter != route_entry->routeBySsn.end() ) {
      smsc_log_debug(_logger, "GTTranslationTable::findRouteBySSN::: entry for ssn=%d was found, routeId=%s", destination_ssn, iter->second.c_str());
      *route_id = iter->second;
      return true;
    }
  }
  return false;
}

void
GTTranslationTable::addTranslationEntry(const std::string& gt_mask_value,
                                        const std::string& route_id,
                                        bool route_to_MTP3)
{
  smsc_log_info(_logger, "GTTranslationTable::addTranslationEntry::: add route entry=[gt_mask_value=[%s],route_id=[%s],route_to_MTP3=[%s]]",
                gt_mask_value.c_str(), route_id.c_str(), route_to_MTP3? "true" : "false");
  RouteTableEntry *routeEntry=NULL;
  {
    smsc::core::synchronization::MutexGuard routingTableSynchronize(_routingTableLock);

    if ( _routingTable.Find(gt_mask_value.c_str(), routeEntry) ) {
      smsc::core::synchronization::MutexGuard routeEntrySynchronize(routeEntry->routingEntryLock);

      routeEntry->routeId = route_id;
    } else
      _routingTable.Insert(gt_mask_value.c_str(), new RouteTableEntry(route_id,
                                                                      route_to_MTP3));
  }
}

void
GTTranslationTable::addTranslationEntry(const std::string& gt_mask_value,
                                        uint8_t destination_ssn,
                                        const std::string& route_id)
{
  smsc_log_info(_logger, "GTTranslationTable::addTranslationEntry::: add route entry=[gt_mask_value=[%s],route_id=[%s],route_to_MTP3=[false]]",
                gt_mask_value.c_str(), route_id.c_str());
  RouteTableEntry *routeEntry=NULL;
  {
    smsc::core::synchronization::MutexGuard routingTableSynchronize(_routingTableLock);

    if ( _routingTable.Find(gt_mask_value.c_str(), routeEntry) ) {
      smsc::core::synchronization::MutexGuard routeEntrySynchronize(routeEntry->routingEntryLock);

      routeEntry->routeBySsn.insert(std::make_pair(destination_ssn, route_id));
    } else
      _routingTable.Insert(gt_mask_value.c_str(), new RouteTableEntry(destination_ssn,
                                                                      route_id,
                                                                      false));
  }
}

std::string
GTTranslationTable_Prefix_4_0_1_4::trimGTMaskValue(const std::string& gt_mask_value)
{
  std::string modifiedGtMaskValue(gt_mask_value);
  if ( modifiedGtMaskValue[0] == '+' )
    modifiedGtMaskValue.erase(0,1); // erase '+' symbol
  else {
    std::string::size_type idx = modifiedGtMaskValue.rfind('.');
    if ( idx != std::string::npos )
      modifiedGtMaskValue.erase(0, idx+1);
  }

  return modifiedGtMaskValue;
}

void
GTTranslationTable_Prefix_4_0_1_4::addTranslationEntry(const std::string& gt_mask_value,
                                                       const std::string& route_id,
                                                       bool route_to_MTP3)
{
  GTTranslationTable::addTranslationEntry(trimGTMaskValue(gt_mask_value), route_id,
                                          route_to_MTP3);
}

void
GTTranslationTable_Prefix_4_0_1_4::addTranslationEntry(const std::string& gt_mask_value,
                                                       uint8_t destination_ssn,
                                                       const std::string& route_id)
{
  GTTranslationTable::addTranslationEntry(trimGTMaskValue(gt_mask_value), destination_ssn,
                                          route_id);
}

void
GTTranslationTable_Prefix_4_0_1_0::addTranslationEntry(const std::string& gt_mask_value,
                                                       const std::string& route_id,
                                                       bool route_to_MTP3)
{
  GTTranslationTable::addTranslationEntry(gt_mask_value, route_id, route_to_MTP3);
}

void
GTTranslationTable_Prefix_4_0_1_0::addTranslationEntry(const std::string& gt_mask_value,
                                                       uint8_t destination_ssn,
                                                       const std::string& route_id)
{
  GTTranslationTable::addTranslationEntry(gt_mask_value, destination_ssn, route_id);
}

}}}}}
