#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_ROUTER_GTTRANSLATIONTABLE_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_ROUTER_GTTRANSLATIONTABLE_HPP__

# include <map>
# include <utility>

# include "logger/Logger.h"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/utilx/PrefixTree.hpp"
# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/router/GTTranslator.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace router {

class GTTranslationTable {
public:
  GTTranslationTable();

  GTTranslationResult translate(const eyeline::sccp::SCCPAddress& address);

  virtual void addTranslationEntry(const std::string& gt_mask_value,
                                   const std::string& route_id,
                                   bool route_to_MTP3 = false);

  virtual void addTranslationEntry(const std::string& gt_mask_value,
                                   uint8_t destination_ssn,
                                   const std::string& route_id);

protected:
  smsc::logger::Logger* _logger;

  struct RouteTableEntry {
    RouteTableEntry(const std::string& route_id, bool route_to_MTP3)
    : routeId(route_id), routeToMTP3(route_to_MTP3)
    {}

    RouteTableEntry(uint8_t ssn, const std::string& route_id, bool route_to_MTP3)
    : routeToMTP3(false)
    {
      routeBySsn.insert(std::make_pair(ssn, route_id));
    }

    std::string routeId;
    bool routeToMTP3;

    std::map<uint8_t/*ssn*/, std::string/*route_id*/> routeBySsn;
    mutable smsc::core::synchronization::Mutex routingEntryLock;
  };
  typedef utilx::PrefixTree<RouteTableEntry*> rtable_t;

  rtable_t _routingTable;

  smsc::core::synchronization::Mutex _routingTableLock;

  bool translationResultIncludesSsn(const RouteTableEntry* route_entry);

  bool findRouteBySSN(std::string* route_id,
                      const RouteTableEntry* route_entry,
                      uint8_t destination_ssn);

private:
  GTTranslationTable(const GTTranslationTable& rhs);
  GTTranslationTable& operator=(const GTTranslationTable& rhs);
};

class GTTranslationTable_Prefix_4_0_1_4 : public GTTranslationTable {
public:
  virtual void addTranslationEntry(const std::string& gt_mask_value,
                                   const std::string& route_id,
                                   bool route_to_MTP3 = false);

  virtual void addTranslationEntry(const std::string& gt_mask_value,
                                   uint8_t destination_ssn,
                                   const std::string& route_id);
private:
  std::string trimGTMaskValue(const std::string& gt_mask_value);
};

class GTTranslationTable_Prefix_4_0_1_0 : public GTTranslationTable {
public:
  virtual void addTranslationEntry(const std::string& gt_mask_value,
                                   const std::string& route_id,
                                   bool route_to_MTP3 = false);

  virtual void addTranslationEntry(const std::string& gt_mask_value,
                                   uint8_t destination_ssn,
                                   const std::string& route_id);
};

}}}}}

#endif
