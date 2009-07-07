#include "SMPPSubsystem.hpp"
#include "SMPPMessageHandlersFactory.hpp"
#include "eyeline/load_balancer/io_subsystem/MessageHandlersFactoryRegistry.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

void
SMPPSubsystem::initialize()
{
  io_subsystem::MessageHandlersFactoryRegistry::getInstance().registerMessageHandlersFactory(new SMPPMessageHandlersFactory());

}

}}}}
