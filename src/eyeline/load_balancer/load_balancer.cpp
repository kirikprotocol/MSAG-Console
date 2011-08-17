#include <unistd.h>
#include <signal.h>

#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

#include "eyeline/load_balancer/io_subsystem/IOSubsystem.hpp"
#include "eyeline/load_balancer/protocols/smpp/SMPPSubsystem.hpp"
#include "eyeline/load_balancer/protocols/smpp/ProtocolStates.hpp"
#include "eyeline/load_balancer/protocols/smpp/SmeRegistry.hpp"

#include "eyeline/load_balancer/io_subsystem/SwitchingTable.hpp"
#include "eyeline/load_balancer/io_subsystem/TimeoutMonitor.hpp"
#include "eyeline/load_balancer/io_subsystem/IOProcessorMgrRegistry.hpp"
#include "eyeline/load_balancer/io_subsystem/MessageHandlersFactoryRegistry.hpp"

#include "eyeline/load_balancer/cluster/ClusterSubsystem.hpp"

extern "C"
void
sig_handler(int signo)
{
  return;
}

static void initialize()
{
  smsc::logger::Logger::Init();
  eyeline::load_balancer::protocols::smpp::SMPP_NotConnected::init();
  eyeline::load_balancer::protocols::smpp::SMPP_Connected::init();
  eyeline::load_balancer::protocols::smpp::SMPP_Binding_TX::init();
  eyeline::load_balancer::protocols::smpp::SMPP_Bound_TX::init();
  eyeline::load_balancer::protocols::smpp::SMPP_Binding_RX::init();
  eyeline::load_balancer::protocols::smpp::SMPP_Bound_RX::init();
  eyeline::load_balancer::protocols::smpp::SMPP_Binding_TRX::init();
  eyeline::load_balancer::protocols::smpp::SMPP_Bound_TRX::init();
  eyeline::load_balancer::protocols::smpp::SMPP_Unbound::init();
  eyeline::load_balancer::protocols::smpp::SmeRegistry::init();
  eyeline::load_balancer::io_subsystem::SwitchingTable::init();
  eyeline::load_balancer::io_subsystem::IOProcessorMgrRegistry::init();
  eyeline::load_balancer::io_subsystem::MessageHandlersFactoryRegistry::init();
  eyeline::load_balancer::io_subsystem::TimeoutMonitor::init();
}

extern "C"
int main(int argc, char** argv)
{
  initialize();
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("init");
  smsc_log_error(logger, "\n");
  smsc_log_error(logger, "starting load balancer.");

  const char* cfgFile;
  if ( argc < 2 )
    cfgFile = "config.xml";
  else
    cfgFile = argv[1];

  try {
    smsc::util::config::Manager::init(cfgFile);
    smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();

    smsc::util::config::ConfigView loadBalancerConfigView(manager, "load_balancer");
    if ( !loadBalancerConfigView.findSubSection("smpp") ) {
      smsc_log_error(logger, "invalid config format: subsection 'smpp' is absent in section 'load_balancer' [%s]. Terminate.");
      return 1;
    }
    std::auto_ptr<smsc::util::config::ConfigView>
      smppConfigView(loadBalancerConfigView.getSubConfig("smpp"));

    if ( !loadBalancerConfigView.findSubSection("cluster") ) {
      smsc_log_error(logger, "invalid config format: subsection 'cluster' is absent in section 'load_balancer' [%s]. Terminate.");
      return 1;
    }
    std::auto_ptr<smsc::util::config::ConfigView>
      clusterConfigView(loadBalancerConfigView.getSubConfig("cluster"));

    sigset(SIGTERM,sig_handler);

    eyeline::load_balancer::protocols::smpp::SMPPSubsystem smppSubsystem;
    smppSubsystem.initialize();

    eyeline::load_balancer::io_subsystem::IOSubsystem ioSubsystem;
    ioSubsystem.initialize(*smppConfigView);
    ioSubsystem.start();

    eyeline::load_balancer::cluster::ClusterSubsystem clusterSubsystem;
    clusterSubsystem.initialize(*clusterConfigView);
    clusterSubsystem.start();

    pause();

    clusterSubsystem.stop();
    ioSubsystem.stop();
  } catch(std::exception& ex) {
    smsc_log_error(logger, "caught unexpected exception [%s]. Terminate.", ex.what());
    return 1;
  }
  smsc_log_error(logger, "load balancer shutdown complete.\n");
  return 0;
}
