#include <logger/Logger.h>
#include <util/config/Manager.h>
#include "Configuration.hpp"
#include <exception>
#include <string.h>
#include <signal.h>

static smsc::logger::Logger*
initLogger(const char* moduleName)
{
  smsc::logger::Logger::Init();
  return smsc::logger::Logger::getInstance(moduleName);
}

smsc::logger::Logger* dmplxlog=initLogger("dmplx");

#ifdef __cplusplus
int main(int argc, char* argv[]);
#endif

void multiplexer();

static const char* configFile = "conf/config.xml";
static const char* module_version = "$Revision$";
static const char* module_target = "$Name$";

static void
printUsage(const char* moduleName)
{
  printf("Usage: %s [-h]|[-c config_file]|[-v]\n", moduleName);
}

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

    configuration.listenPort=configMgr.getInt("InConnect.incomingPort");
    configuration.smscHost=configMgr.getString("OutConnect.smscHost");
    configuration.smscPort=configMgr.getInt("OutConnect.smscPort");

    configuration.enqLinkPeriod=configMgr.getInt("LinkControl.EnqLinkPeriod");
    configuration.selectTimeOut=configMgr.getInt("LinkControl.SelectTimeOut");

    smsc::util::config::CStrSet *sectionList
      = configMgr.getChildSectionNames("Authentication");
    if ( sectionList ) {
      for (smsc::util::config::CStrSet::iterator iter=sectionList->begin();
           iter != sectionList->end(); ++iter) {
        const std::string systemIdParam = *iter + std::string(".systemId");
        const std::string passwdParam = *iter + std::string(".password");
        configuration.sysid_Passwd_map.insert(std::make_pair(configMgr.getString(systemIdParam.c_str()),configMgr.getString(passwdParam.c_str())));
      }
    } else {
      fprintf(stderr, "<Authentication> section is absent in configuration file");
      return 1;
    }

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
