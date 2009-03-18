#ifndef __EYELINE_SUA_SUALAYER_SUASTACK_RCREGISTRY_HPP__
# define __EYELINE_SUA_SUALAYER_SUASTACK_RCREGISTRY_HPP__

# include <logger/Logger.h>
# include <core/buffers/Hash.hpp>
# include <core/synchronization/Mutex.hpp>
# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/communication/sua_messages/SuaTLV.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

class RCRegistry : public utilx::Singleton<RCRegistry> {
public:
  RCRegistry();
  communication::sua_messages::TLV_RoutingContext getRoutingContext(const communication::LinkId& outLinkId);
  void insert(const communication::LinkId& linkId, const communication::sua_messages::TLV_RoutingContext& rc);
  void remove(const communication::LinkId& linkId);
private:
  smsc::core::synchronization::Mutex _lock;
  smsc::core::buffers::Hash<communication::sua_messages::TLV_RoutingContext> _registredRoutingContexts;
  smsc::logger::Logger* _logger;
};

}}}}

#endif
