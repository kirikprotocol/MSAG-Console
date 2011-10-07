#include <vector>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/resource.h>
#include "core/buffers/Hash.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "util/config/ConfigException.h"
#include "mcisme/misscallhd/callproc.hpp"

using std::vector;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;
using smsc::core::buffers::Hash;
using smsc::misscall::MissedCallProcessor;
using smsc::misscall::MissedCallListener;
using smsc::misscall::MissedCallEvent;
using smsc::misscall::ReleaseSettings;
using smsc::misscall::REDIREC_RULE_STRATEGY;
using smsc::misscall::Circuits;
using smsc::misscall::Rule;
using smsc::logger::Logger;

static Logger* logger = 0;
static int  userid = 41; //common part user id
static int userinstance = 1;
//static int ISUP_INSTANCE = 1;
//static char cpMgrHostAndPort[30] = {0};
std::string isupinstances;
static std::string cpMgrHostAndPort;

class Listener: public MissedCallListener{
  public:
      virtual void missed(const MissedCallEvent& event) {
        smsc_log_debug(logger, "event: %s->%s",event.from.c_str(),event.to.c_str());
      };
};
extern "C"
static void sighandler( int signal ) {
  MissedCallProcessor *p = MissedCallProcessor::instance();
  if (p)
  {
    p->removeMissedCallListener();
    p->stop();
  }
}
static ReleaseSettings relCauses = {
/* strategy */ REDIREC_RULE_STRATEGY,
/* busy     */ 0x11 /* called user busy    */, 0x01 /* inform              */,
/* no reply */ 0x13 /* no answer from user */, 0x01 /* inform              */,
/* uncond   */ 0x15 /* call rejected       */, 0x00 /* don't inform        */,
/* absent   */ 0x14 /* subscriber absent   */, 0x01 /* inform              */,
/* detach   */ 0x14 /* subscriber absent   */, 0x01 /* inform              */,
/* other    */ 0x15 /* call rejected       */, 0x01 /* inform just in case */,
/* skip uca */ false
};
static struct Circuits channel = {
  /* ts  */ 0xFFFFFFFE,
  /* hsn */ 0,
  /* spn */ 1
};
Hash<Circuits> circuitsMap;
static struct Rule rule = {
  /* rx       */ "//",
  /* name     */ "all",
  /* priority */ 100,
  /* cause    */ 20,
  /* inform   */ 1
};
vector<Rule> rules;
class ToolConfig {
  private:
    Logger* logger;
  public:
    ToolConfig(Logger* _logger):logger(_logger){}
    void read(Manager& manager)
    {

      if (!manager.findSection("tieto"))
        throw ConfigException("\'tieto\' section is missed");

      ConfigView tietoConfig(manager, "tieto");

      try { cpMgrHostAndPort = tietoConfig.getInt("userid");
      } catch (ConfigException& exc) {
        throw ConfigException("\'userid\' is unknown or missing");
      }

      try { userinstance = tietoConfig.getInt("userinstance");
      } catch (ConfigException& exc) {
        throw ConfigException("\'userinstance\' is unknown or missing");
      }

      try {
        cpMgrHostAndPort = tietoConfig.getString("cpMgrHostAndPort");
      } catch (ConfigException& exc) {
        throw ConfigException("\'cpMgrHostAndPort\' is unknown or missing");
      }

      try {
        isupinstances = tietoConfig.getString("remoteInstancies");
      } catch (ConfigException& exc) {
        throw ConfigException("\'remoteInstancies\' is unknown or missing");
      }
    }
};
int main (int argc, char *argv[])
{
  goto nodaemon;
  if (getppid() != 1)
  {
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    int dpid;
    if ((dpid = fork()) != 0)
    {
      printf("%s pid=%d\n",argv[0],dpid);
      exit(0);
    }
    setsid();
  }
  struct rlimit flim;
  getrlimit(RLIMIT_NOFILE, &flim);
  for (int fd = 0; fd < flim.rlim_max; fd ++)
  {
    close(fd);
  }
nodaemon:
  Logger::Init();
  logger = Logger::getInstance("smsc.misscall");


  Manager::init("config.xml");
  Manager& manager = Manager::getInstance();
  ToolConfig config(logger);
  config.read(manager);
  ToolConfig sccpcfg(logger);
  sccpcfg.read(manager);


  Listener listener;
  MissedCallProcessor *processor = MissedCallProcessor::instance();
  processor->configure(userid,userinstance,cpMgrHostAndPort,isupinstances);
  processor->setReleaseSettings(relCauses);
  circuitsMap.Insert("sample",channel);
  processor->setCircuits(circuitsMap);
  rules.push_back(rule);
  processor->setRules(rules);
  processor->addMissedCallListener(&listener);
  sigset( SIGTERM, sighandler );
  processor->run();
}
