#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_RCREGISTRY_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_RCREGISTRY_HPP__

# include "logger/Logger.h"
# include "core/buffers/Hash.hpp"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {

class RCRegistry : public utilx::Singleton<RCRegistry> {
public:
  RCRegistry();
  common::TLV_RoutingContext getRoutingContext(const common::LinkId& out_link_id);
  void insert(const common::LinkId& link_id, const common::TLV_RoutingContext& rc);
  void remove(const common::LinkId& link_id);
private:
  smsc::core::synchronization::Mutex _lock;
  smsc::core::buffers::Hash<common::TLV_RoutingContext> _registredRoutingContexts;
  smsc::logger::Logger* _logger;
};

}}}}

#endif
