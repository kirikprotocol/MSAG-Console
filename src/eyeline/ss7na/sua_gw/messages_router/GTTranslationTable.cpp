#include "eyeline/ss7na/common/types.hpp"
#include "GTTranslationTable.hpp"
#include "eyeline/ss7na/common/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace messages_router {

GTTranslationTable::GTTranslationTable()
  : _logger(smsc::logger::Logger::getInstance("msg_rout"))
{}

common::LinkId
GTTranslationTable::translate(const sua_stack::messages::TLV_DestinationAddress& address)
{
  const sua_stack::messages::GlobalTitle& gt = address.getGlobalTitle().getGlobalTitleValue();

  const std::string& gtValue = gt.getGlobalTitleDigits();

  const sua_stack::messages::TLV_SSN& ssn = address.getSSN();
  uint8_t ssnValue=0;

  if ( ssn.isSetValue() )
    ssnValue = ssn.getValue();

  smsc_log_debug(_logger, "GTTranslationTable::translate::: try find output LinkId for routing key=[gt=%s,ssn=%d]", gtValue.c_str(), ssnValue);

  RouteTableEntry *routeEntry=NULL;
  bool routeWasFound = true;

  smsc_log_debug(_logger, "GTTranslationTable::translate::: call _routingTable.FindPrefix(%s)", gtValue.c_str());

  common::LinkId outLinkSetId;
  bool searchResult;
  {
    smsc::core::synchronization::MutexGuard synchronize(_routingTableLock);
    searchResult = _routingTable.FindPrefix(gtValue.c_str(), routeEntry);
  }
  if ( !searchResult ) {
    outLinkSetId = getDefaultRoute();
    routeWasFound = true;
  } else {
    smsc_log_debug(_logger, "GTTranslationTable::translate::: found entry for gt=[%s]", gtValue.c_str());
    findRouteBySSN(routeEntry, ssnValue, &outLinkSetId);

    if ( outLinkSetId == common::LinkId() )
      routeWasFound = false;
  }

  if ( routeWasFound )
    smsc_log_info(_logger, "GTTranslationTable::translate::: found route with LinkSetId=[%s] for routing key=[gt=%s,ssn=%d]", outLinkSetId.getValue().c_str(), gtValue.c_str(), ssnValue);
  else {
    smsc_log_info(_logger, "GTTranslationTable::translate::: route not found for routing key=[gt=%s,ssn=%d]", gtValue.c_str(), ssnValue);
    throw common::TranslationFailure(common::NO_ADDR_TRANSLATION_FOR_SPECIFIC_ADDR, "GTTranslationTable::translate::: route not found");
  }

  return outLinkSetId;
}

void
GTTranslationTable::findRouteBySSN(const RouteTableEntry* routeEntry,
                                   uint8_t destinationSSN,
                                   common::LinkId* outLinkSetId)
{
  smsc::core::synchronization::MutexGuard synchronize(routeEntry->routingEntryLock);
  smsc_log_debug(_logger, "GTTranslationTable::findRouteBySSN::: try match for ssn=%d", destinationSSN);
  std::map<uint8_t, common::LinkId>::const_iterator iter = routeEntry->routeBySsn.find(destinationSSN);
  if ( iter != routeEntry->routeBySsn.end() ) {
    smsc_log_debug(_logger, "GTTranslationTable::findRouteBySSN::: entry for ssn=%d was found, linkSetId=%s", destinationSSN, iter->second.getValue().c_str());
    if ( outLinkSetId )
      *outLinkSetId = iter->second;
    return;
  }

  if ( outLinkSetId ) {
    smsc_log_debug(_logger, "GTTranslationTable::findRouteBySSN::: get default linksetid=%s",routeEntry->linkSetIdForRouteBySSN_Value_0.getValue().c_str());
    *outLinkSetId = routeEntry->linkSetIdForRouteBySSN_Value_0; // linkId corresponding to route for SSN=0
  }
  return;
}

common::LinkId
GTTranslationTable::getDefaultRoute()
{
  if ( _isSetDefaultRoute ) {
    smsc_log_info(_logger, "GTTranslationTable::getDefaultRoute::: found default route with LinkSetId=[%s]", _defaultRouteLinkSetId.getValue().c_str());
    return _defaultRouteLinkSetId;
  } else
    throw common::TranslationFailure(common::NO_ADDR_TRANSLATION_FOR_SPECIFIC_ADDR, "GTTranslationTable::getDefaultRoute::: there isn't default route");
}

void
GTTranslationTable::addTranslationEntry(const std::string& gtMaskValue,
                                        uint8_t destinationSSN,
                                        const common::LinkId& linkSetId)
{
  smsc_log_info(_logger, "GTTranslationTable::addTranslationEntry::: add route entry=[gtMaskValue=[%s],ssn=[%d],linkSetId=[%s]]", gtMaskValue.c_str(), destinationSSN, linkSetId.getValue().c_str());
  RouteTableEntry *routeEntry=NULL;
  {
    smsc::core::synchronization::MutexGuard routingTableSynchronize(_routingTableLock);

    if ( _routingTable.Find(gtMaskValue.c_str(), routeEntry) ) {
      smsc::core::synchronization::MutexGuard routeEntrySynchronize(routeEntry->routingEntryLock);

      if ( destinationSSN == 0 )
        routeEntry->linkSetIdForRouteBySSN_Value_0 = linkSetId;
      else
        routeEntry->routeBySsn.insert(std::make_pair(destinationSSN, linkSetId));
    } else
      _routingTable.Insert(gtMaskValue.c_str(), new RouteTableEntry(destinationSSN, linkSetId));
  }
}

common::LinkId
GTTranslationTable::_defaultRouteLinkSetId;

bool
GTTranslationTable::_isSetDefaultRoute = false;

void
GTTranslationTable::addDefaultTranslationEntry(const common::LinkId& linkSetId)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("msg_rout");

  smsc_log_info(logger, "GTTranslationTable::addDefaultTranslationEntry::: add default route entry=[linkSetId=[%s]]", linkSetId.getValue().c_str());
  if ( !_isSetDefaultRoute ) {
    _isSetDefaultRoute = true;
    _defaultRouteLinkSetId = linkSetId;
  } else
    throw smsc::util::Exception("GTTranslationTable::addDefaultTranslationEntry::: default route has been already set");
}

void
GTTranslationTable_Prefix_4_0_1_4::addTranslationEntry(const std::string& gtMaskValue,
                                                       uint8_t destinationSSN,
                                                       const common::LinkId& linkSetId)
{
  std::string modifiedGtMaskValue(gtMaskValue);
  if ( modifiedGtMaskValue[0] == '+' )
    modifiedGtMaskValue.erase(0,1); // erase '+' symbol
  else {
    std::string::size_type idx = modifiedGtMaskValue.rfind('.');
    if ( idx != std::string::npos )
      modifiedGtMaskValue.erase(0, idx+1);
  }
  GTTranslationTable::addTranslationEntry(modifiedGtMaskValue, destinationSSN, linkSetId);
}

void
GTTranslationTable_Prefix_4_0_1_0::addTranslationEntry(const std::string& gtMaskValue,
                                                       uint8_t destinationSSN,
                                                       const common::LinkId& linkSetId)
{
  GTTranslationTable::addTranslationEntry(gtMaskValue, destinationSSN, linkSetId);
}

}}}}
