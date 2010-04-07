#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_MSUPROCESSOR_ROUTINGTABLE_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_MSUPROCESSOR_ROUTINGTABLE_HPP__

# include <sys/types.h>

# include "logger/Logger.h"
# include "core/buffers/IntHash.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/types.hpp"
# include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace msu_processor {

class RoutingTable {
public:
  RoutingTable()
  : _logger(smsc::logger::Logger::getInstance("msu_proc"))
  {}

  void addRoute(common::point_code_t dpc, const common::LinkId& dst_link_id);
  void removeRoute(common::point_code_t dpc);
  const common::LinkId& getRoute(common::point_code_t dpc);
private:
  smsc::logger::Logger* _logger;
  smsc::core::buffers::IntHash<common::LinkId> _routingTable;
  smsc::core::synchronization::Mutex _lock;
};

}}}}}

#endif
