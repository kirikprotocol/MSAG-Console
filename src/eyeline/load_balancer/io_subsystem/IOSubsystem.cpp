#include <memory>

#include "IOSubsystem.hpp"
#include "IOParameters.hpp"
#include "SwitchingTable.hpp"
#include "IOProcessorMgrTmpl.hpp"
#include "IOProcessorMgrRegistry.hpp"
#include "SwitchCircuitControllerTmpl.hpp"
#include "TimeoutMonitor.hpp"
#include "eyeline/load_balancer/balancers/MSAGLinkSet.hpp"
#include "eyeline/load_balancer/balancers/SMSCLinkSet.hpp"
#include "eyeline/load_balancer/protocols/smpp/SMPPConnection.hpp"
#include "eyeline/load_balancer/protocols/smpp/SMPPIOProcessor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

IOSubsystem::IOSubsystem()
  : _logger(smsc::logger::Logger::getInstance("io_subsystem"))
  {}

void
IOSubsystem::initialize(const smsc::util::config::ConfigView& smpp_cfg_entry)
{
  std::auto_ptr<std::set<std::string> > setGuard(smpp_cfg_entry.getShortSectionNames());
  unsigned ioProcMgrid = 0;
  for(std::set<std::string>::iterator iter = setGuard->begin(), end_iter = setGuard->end();
      iter != end_iter; ++ iter) {
    std::string switchEntryName = *iter;
    smsc_log_debug(_logger, "IOSubsystem::initialize::: processing config section '%s'",
                   switchEntryName.c_str());
    std::auto_ptr<smsc::util::config::ConfigView> switchEntry(smpp_cfg_entry.getSubConfig(switchEntryName.c_str()));
    std::string linksetPrefix = switchEntry->getString("linkset_prefix");
    if ( switchEntry->findSubSection("out_links") ) {
      std::auto_ptr<smsc::util::config::ConfigView> outLinksCfgSection(switchEntry->getSubConfig("out_links"));

      std::string linkUsagePolicy = outLinksCfgSection->getString("link_usage_policy");
      smsc_log_debug(_logger, "IOSubsystem::initialize::: processing out link definitions: link_usage_policy='%s'", linkUsagePolicy.c_str());
      SwitchCircuitController* switchCircuitCtrl;
      if ( linkUsagePolicy == "round-robin" )
        switchCircuitCtrl =
          new SwitchCircuitControllerTmpl<protocols::smpp::SMPPConnection, balancers::SMSCLinkSet>(linksetPrefix,
              *outLinksCfgSection,
              io_subsystem::SwitchingTable::getInstance());
      else if ( linkUsagePolicy == "msag-specific" )
        switchCircuitCtrl =
          new SwitchCircuitControllerTmpl<protocols::smpp::SMPPConnection, balancers::MSAGLinkSet>(linksetPrefix,
              *outLinksCfgSection,
              io_subsystem::SwitchingTable::getInstance());
      else {
        smsc_log_error(_logger, "IOSubsystem::initialize::: invalid link_usage_policy parameter value = '%s', skip it", linkUsagePolicy.c_str());
        continue;
      }
      std::auto_ptr<IOProcessorMgr>
        ioProcMgr(new IOProcessorMgrTmpl<protocols::smpp::SMPPIOProcessor>(ioProcMgrid++,
                                                                           switchEntry->getString("listening_host"),
                                                                           switchEntry->getInt("listening_port"),
                                                                           switchCircuitCtrl,
                                                                           switchEntry->getInt("max_newconn_event_handlers"),
                                                                           switchEntry->getInt("max_newconn_events_queue_sz")));
      IOParameters ioParameters(switchEntry->getInt("max_events_queue_sz"),
                                switchEntry->getInt("reconnect_attempt_period"),
                                switchEntry->getInt("max_out_packets_queue_sz"),
                                switchEntry->getInt("max_num_of_event_processors"),
                                switchEntry->getInt("max_sockets_per_ioprocessor"));
      ioProcMgr->setParameters(ioParameters);
      smsc_log_info(_logger, "set config parameters='%s' for IOProcessoMgr object with id=%d",
                    ioParameters.toString().c_str(), ioProcMgr->getId());
      IOProcessorMgrRegistry::getInstance().registerIOProcMgr(ioProcMgr.release());
    } else
      smsc_log_error(_logger, "IOSubsystem::initialize::: 'out_links' section is absent");

  }
}

void
IOSubsystem::start()
{
  smsc_log_info(_logger, "IOSubsystem::start::: try start IOProcessorMgr objects");
  IOProcessorMgrRegistry::Iterator ioProcMgrIter =
    IOProcessorMgrRegistry::getInstance().getIterator();
  while (ioProcMgrIter.hasElement()) {
    ioProcMgrIter.getCurrentElement()->startup();
    ioProcMgrIter.next();
  }
  smsc_log_info(_logger, "IOSubsystem::start::: try start TimeoutMonitor objects");
  TimeoutMonitor::getInstance().Start();
}

void
IOSubsystem::stop()
{
  IOProcessorMgr* ioProcMgr = IOProcessorMgrRegistry::getInstance().unregisterNextIOProcMgr();
  while (ioProcMgr) {
    ioProcMgr->shutdown();
    delete ioProcMgr;
    ioProcMgr = IOProcessorMgrRegistry::getInstance().unregisterNextIOProcMgr();
  }
}

}}}

