#ifndef __EYELINE_SS7NA_SUAGW_MESSAGESROUTER_GTTRANSLATIONTABLE_HPP__
# define __EYELINE_SS7NA_SUAGW_MESSAGESROUTER_GTTRANSLATIONTABLE_HPP__

# include <map>
# include <utility>

# include "logger/Logger.h"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/PrefixTree.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace messages_router {

class GTTranslationTable {
public:
  GTTranslationTable();
  common::LinkId translate(const sua_stack::messages::TLV_DestinationAddress& address);

  virtual void addTranslationEntry(const std::string& gtMaskValue,
                                   uint8_t destinationSSN,
                                   const common::LinkId& linkSetId);

  static void addDefaultTranslationEntry(const common::LinkId& linkSetId);

protected:
  smsc::logger::Logger* _logger;

  struct RouteTableEntry {
    RouteTableEntry(uint8_t ssn, const common::LinkId& aLinkSetId) {
      if ( ssn == 0 )
        linkSetIdForRouteBySSN_Value_0 = aLinkSetId;
      else
        routeBySsn.insert(std::make_pair(ssn, aLinkSetId));
    }

    common::LinkId linkSetIdForRouteBySSN_Value_0;
    std::map<uint8_t/*ssn*/, common::LinkId/*linkId*/> routeBySsn;
    mutable smsc::core::synchronization::Mutex routingEntryLock;
  };
  typedef utilx::PrefixTree<RouteTableEntry*> rtable_t;

  rtable_t _routingTable;

  smsc::core::synchronization::Mutex _routingTableLock;

  static common::LinkId _defaultRouteLinkSetId;
  static bool _isSetDefaultRoute;

  void findRouteBySSN(const RouteTableEntry* routeEntry,
                      uint8_t destinationSSN,
                      common::LinkId* outLinkSetId=NULL);

  common::LinkId getDefaultRoute();

private:
  GTTranslationTable(const GTTranslationTable& rhs);
  GTTranslationTable& operator=(const GTTranslationTable& rhs);
};

class GTTranslationTable_Prefix_4_0_1_4 : public GTTranslationTable {
public:
  virtual void addTranslationEntry(const std::string& gtMaskValue,
                                   uint8_t destinationSSN,
                                   const common::LinkId& linkSetId);
};

class GTTranslationTable_Prefix_4_0_1_0 : public GTTranslationTable {
public:
  virtual void addTranslationEntry(const std::string& gtMaskValue,
                                   uint8_t destinationSSN,
                                   const common::LinkId& linkSetId);
};

}}}}

#endif
