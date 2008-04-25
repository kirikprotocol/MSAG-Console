#ifndef __SUA_SUALAYER_SUASTACK_RCREGISTRY_HPP__
# define __SUA_SUALAYER_SUASTACK_RCREGISTRY_HPP__ 1

# include <logger/Logger.h>
# include <core/buffers/Hash.hpp>
# include <core/synchronization/Mutex.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/utilx/Singleton.hpp>

namespace sua_stack {

class RCRegistry : public utilx::Singleton<RCRegistry> {
public:
  RCRegistry();
  sua_messages::TLV_RoutingContext getRoutingContext(const communication::LinkId& outLinkId);
  void insert(const communication::LinkId& linkId, const sua_messages::TLV_RoutingContext& rc);
  void remove(const communication::LinkId& linkId);
private:
  smsc::core::synchronization::Mutex _lock;
  smsc::core::buffers::Hash<sua_messages::TLV_RoutingContext> _registredRoutingContexts;
  smsc::logger::Logger* _logger;
};

}

#endif
