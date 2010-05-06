#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_MSUPROCESSOR_ROUTER_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_MSUPROCESSOR_ROUTER_HPP__

# include "logger/Logger.h"
# include "core/buffers/IntHash.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/Exception.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/msu_processor/RoutingTable.hpp"
# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace msu_processor {

class Router : public utilx::Singleton<Router> {
public:
  Router()
  : _logger(smsc::logger::Logger::getInstance("msu_proc"))
  {}

  common::LinkId route(common::point_code_t opc, common::point_code_t dpc,
                       const common::LinkId& src_link_id) const {
    throw common::MTP3RouteNotFound("Router::route::: can't route transit message for opc=%d/dpc=%d",
                                    opc, dpc);
  }

  common::LinkId route(common::point_code_t lpc, common::point_code_t dpc) const;

  void addRoutingTable(common::point_code_t lpc, RoutingTable* routing_table);
  RoutingTable* getRoutingTable(common::point_code_t lpc) const;
  bool removeRoutingTable(common::point_code_t lpc);

private:
  smsc::core::buffers::IntHash<RoutingTable*> _routingTables;
  smsc::logger::Logger* _logger;
};

}}}}}

#endif
