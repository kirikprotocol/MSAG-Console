#ifndef __SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATIONTABLE_HPP__
# define __SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATIONTABLE_HPP__

# include <map>
# include <utility>
# include <logger/Logger.h>
# include <core/synchronization/Mutex.hpp>
# include <sua/utilx/Singleton.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>
# include <sua/utilx/PrefixTree.hpp>

namespace messages_router {

class GTTranslationTable {
public:
  GTTranslationTable();
  communication::LinkId translate(const sua_messages::TLV_DestinationAddress& address);

  virtual void addTranslationEntry(const std::string& gtMaskValue,
                                   uint8_t destinationSSN,
                                   const communication::LinkId& linkSetId);

  static void addDefaultTranslationEntry(const communication::LinkId& linkSetId);

protected:
  smsc::logger::Logger* _logger;

  struct RouteTableEntry {
    RouteTableEntry(uint8_t ssn, const communication::LinkId& aLinkSetId) {
      if ( ssn == 0 )
        linkSetIdForRouteBySSN_Value_0 = aLinkSetId;
      else
        routeBySsn.insert(std::make_pair(ssn, aLinkSetId));
    }

    communication::LinkId linkSetIdForRouteBySSN_Value_0;
    std::map<uint8_t/*ssn*/, communication::LinkId/*linkId*/> routeBySsn;
    mutable smsc::core::synchronization::Mutex routingEntryLock;
  };
  typedef utilx::PrefixTree<RouteTableEntry*> rtable_t;

  rtable_t _routingTable;

  smsc::core::synchronization::Mutex _routingTableLock;

  static communication::LinkId _defaultRouteLinkSetId;
  static bool _isSetDefaultRoute;

  void findRouteBySSN(const RouteTableEntry* routeEntry,
                      uint8_t destinationSSN,
                      communication::LinkId* outLinkSetId=NULL);

  communication::LinkId getDefaultRoute();

private:
  GTTranslationTable(const GTTranslationTable& rhs);
  GTTranslationTable& operator=(const GTTranslationTable& rhs);
};

class GTTranslationTable_Prefix_4_0_1_4 : public GTTranslationTable {
public:
  virtual void addTranslationEntry(const std::string& gtMaskValue,
                                   uint8_t destinationSSN,
                                   const communication::LinkId& linkSetId);
};

class GTTranslationTable_Prefix_4_0_1_0 : public GTTranslationTable {
public:
  virtual void addTranslationEntry(const std::string& gtMaskValue,
                                   uint8_t destinationSSN,
                                   const communication::LinkId& linkSetId);
};

}

#endif
