#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include "Configuration.hpp"
#include <exception>
#include <string.h>
#include <signal.h>

#include "Publisher.hpp"
#include "UnbindRequest_Subscriber.hpp"
#include "EnquireLinkRequest_Subscriber.hpp"
#include "EnquireLinkResponse_Subscriber.hpp"
#include "BindRequest_Subscriber.hpp"
#include "BindResponse_Subscriber.hpp"
#include "GenericRequest_Subscriber.hpp"
#include "GenericResponse_Subscriber.hpp"
#include "SMPP_MessageFactory.hpp"

#include "SMPP_BindReceiver.hpp"
#include "SMPP_BindReceiver_Resp.hpp"
#include "SMPP_BindTransceiver.hpp"
#include "SMPP_BindTransceiver_Resp.hpp"
#include "SMPP_BindTransmitter.hpp"
#include "SMPP_BindTransmitter_Resp.hpp"
#include "SMPP_EnquireLink.hpp"
#include "SMPP_EnquireLink_Resp.hpp"
#include "SMPP_Unbind.hpp"
#include "SMPP_Unbind_Resp.hpp"

static smsc::logger::Logger*
initLogger(const char* moduleName)
{
  smsc::logger::Logger::Init();
  return smsc::logger::Logger::getInstance(moduleName);
}

smsc::logger::Logger* dmplxlog=initLogger("dmplx");

void multiplexer();

static const char* configFile = "conf/config.xml";
static const char* module_version = "$Revision$";
static const char* module_target = "$Name$";

static void
printUsage(const char* moduleName)
{
  printf("Usage: %s [-h]|[-c config_file]|[-v]\n", moduleName);
}

static void
registerMessageSubscribers()
{
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::GenericRequest_Subscriber());
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::GenericResponse_Subscriber());

  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::EnquireLinkRequest_Subscriber());
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::EnquireLinkResponse_Subscriber());

  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::BindRequest_Subscriber());
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::BindResponse_Subscriber());
  
  smpp_dmplx::Publisher::getInstance().addSubscriber(new smpp_dmplx::UnbindRequest_Subscriber());
}

static void
registerMessagePrototypes()
{
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindReceiver());
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindReceiver_Resp());
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindTransceiver());
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindTransceiver_Resp());
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindTransmitter());
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_BindTransmitter_Resp());
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_EnquireLink());
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_EnquireLink_Resp());
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_Unbind());
  smpp_dmplx::SMPP_MessageFactory::getInstance().registryCreatableMessage(new smpp_dmplx::SMPP_Unbind_Resp());
}

extern "C"
int main(int argc, char* argv[])
{
  try {
    smpp_dmplx::Configuration& 
      configuration = smpp_dmplx::Configuration::getInstance();

    if ( argc > 1 ) {
      const char* opt = argv[1];
      const char* moduleName = strrchr(argv[0],'/');
      if ( moduleName ) ++moduleName;
      if ( !strcmp(opt, "-h") ) {
        printUsage(moduleName);
        return 0;
      } else if ( !strcmp(opt, "-c") ) {
        if ( argc > 2 )
          configFile = argv[2];
        else {
          fprintf(stderr, "config_filename argument is missed\n");
          return 1;
        }
      } else if ( !strcmp(opt, "-v") ) {
        printf("Version: [%s]\nTarget lable: [%s]\n", module_version, module_target);
        return 0;
      } else {
        printUsage(moduleName);
        return 1;
      }
    }
    smsc::util::config::Manager::init(configFile);
    smsc::util::config::Manager& configMgr=smsc::util::config::Manager::getInstance();
    
    smsc::util::config::ConfigView smppdmplxCfg(configMgr, "smppdmplx");

    configuration.listenPort=smppdmplxCfg.getInt("InConnect.incomingPort");
    configuration.smscHost=smppdmplxCfg.getString("OutConnect.smscHost");
    configuration.smscPort=smppdmplxCfg.getInt("OutConnect.smscPort");

    configuration.inactivityPeriod=smppdmplxCfg.getInt("LinkControl.inactivityPeriod");
    configuration.selectTimeOut=smppdmplxCfg.getInt("LinkControl.selectTimeOut");

    if (!smppdmplxCfg.findSubSection("Authentication")) {
      fprintf(stderr, "<Authentication> section is absent in configuration file");
      return 1;
    }

    std::auto_ptr<smsc::util::config::ConfigView> authenticationSections(smppdmplxCfg.getSubConfig("Authentication"));
    std::auto_ptr< std::set<std::string> > setGuard(authenticationSections->getShortSectionNames());
    for (std::set<std::string>::iterator i=setGuard->begin(); i!=setGuard->end(); i++) {
      std::auto_ptr<smsc::util::config::ConfigView> userSection(authenticationSections->getSubConfig((*i).c_str()));
      std::string systemId = userSection->getString("systemId");
      std::string passwd = userSection->getString("password");

      smsc_log_debug(dmplxlog, "DEBUG: systemId=%s,passwd=%s", systemId.c_str(), passwd.c_str());
      configuration.sysid_Passwd_map.insert(std::make_pair(systemId, passwd));
    }

    registerMessageSubscribers();

    registerMessagePrototypes();

    if ( sigset(SIGPIPE, SIG_IGN) == SIG_ERR ) {
      perror ("sigset failed");
      return 1;
    }

    smsc_log_info(dmplxlog,"SME multiplexer was started. Version=[%s]", module_version);

    multiplexer();
  } catch (std::exception& ex) {
    fprintf(stderr, "Catch exception [%s]\n", ex.what());
    return 1;
  }
  return 0;
}
