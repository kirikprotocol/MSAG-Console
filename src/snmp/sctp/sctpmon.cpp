static char const ident[] = "$Id$";
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/resource.h>
#include "util/mirrorfile/mirrorfile.h"
#include "logger/Logger.h"
#include "snmp/sctp/SctpMonitor.hpp"
#include "snmp/sctp/util.hpp"
#include "snmp/sctp/TrapSender.hpp"
#include <string>
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include <util/findConfigFile.h>
#include "core/buffers/XHash.hpp"

using smsc::core::buffers::XHash;
using smsc::snmp::sctp::SctpMonitor;
using smsc::snmp::sctp::TrapSender;
using smsc::snmp::SnmpAgent;
using smsc::snmp::sctp::AssociationChangeListener;
using smsc::snmp::sctp::AssociationChangeEvent;
using smsc::snmp::sctp::getAssociationStateDescription;
using smsc::logger::Logger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using std::string;

struct Association {
  uint16_t said;
  uint8_t  state;
  string   descr;
};


static Logger* logger = 0;
static TrapSender* sender = 0;

class hash_func_said{
public:
  static inline unsigned CalcHash(uint16_t said){
    return (unsigned)said;
  }
};

class Coordinator: public AssociationChangeListener{
  private:
    SctpMonitor *monitor;
    XHash<unsigned,Association*,hash_func_said> assocs;
  public:
    Coordinator() { monitor = 0; }
    void setSctpMonitor(SctpMonitor *_monitor) { monitor = _monitor; }
    void setAssociationDescription(uint16_t said, string description) {
      Association* ptr = findAssociation(said);
      ptr->descr = description;
    }
    Association* findAssociation(uint16_t said) {
      Association** ptr = assocs.GetPtr(said);
      if (ptr)
      {
        return *ptr;
      }
      else
      {
        Association* assoc = new Association();
        assoc->said = said;
        assoc->state = 19;//just fo fun
        char tmp[32] = {0}; int pos = 0;
        pos = snprintf(tmp,sizeof(tmp),"SAID=%d",said);
        assoc->descr = tmp;
        assocs.Insert(said,assoc);
        return assoc;
      }
    }
    int stateCategory(uint8_t state)
    {
      switch(state)
      {
        case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: return -1;
        case 9: return 1;
        default: return 0;
      }      
    }
    virtual void associationChange(AssociationChangeEvent event) {
        Association* assoc = findAssociation(event.said);
        smsc_log_debug(logger, "SAID=%d oSTATE=\"%s\" nSTATE=\"%s\"",
                       event.said,
                       getAssociationStateDescription(assoc->state),
                       getAssociationStateDescription(event.state));
        string status_text = "";
        SnmpAgent::alertStatus  status;
        SnmpAgent::alertSeverity severity;
        string object = "ITP";
        string alarm_id = assoc->descr;
        string text = "";
        string param = "";
        if (stateCategory(event.state) == -1)
        {
          if (monitor) monitor->startAllAssociations();
        }
        if (stateCategory(assoc->state) != stateCategory(event.state))
        {
          if (stateCategory(event.state) == 1) {
            status = SnmpAgent::CLEAR;
            status_text = "Cleared";
            severity = SnmpAgent::NORMAL;
            param = "established";
          }
          if (stateCategory(event.state) == -1) {
            status = SnmpAgent::NEW;
            status_text = "Active";
            severity = SnmpAgent::MAJOR;
            param = "failed";
          }
          text += status_text; text += " "; text += object; text += " "; text += alarm_id; text += " "; text += param;
          if (sender) sender->trap(status,alarm_id.c_str(),object.c_str(),severity,text.c_str());
        }
	assoc->state = event.state;
    }
};

extern "C"
static void sighandler( int signal ) {
  SctpMonitor *m = SctpMonitor::instance();
  if (m)
  {
    m->removeAssociationChangeListener();
    m->stop();
  }
  if (sender) sender->Stop();
}

int main (int argc, char *argv[])
{
#if 0
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
#endif
  Logger::Init();
  smsc::util::config::Manager::init(findConfigFile("config.xml"));
  Manager& manager = Manager::getInstance();
  Coordinator coord;
  ConfigView assocs_cfg(manager, "sctpmon.associations");
  std::auto_ptr< std::set<std::string> > setGuard(assocs_cfg.getShortSectionNames());
  std::set<std::string>* set = setGuard.get();
  for (std::set<std::string>::iterator i=set->begin(); i!=set->end(); i++)
  {
    const char* section = (const char *)i->c_str();
    if (!section || !section[0]) continue;

    std::auto_ptr<ConfigView> addressConfigGuard(assocs_cfg.getSubConfig(section));
    ConfigView* addressConfig = addressConfigGuard.get();

    const char* description = addressConfig->getString("description");
    uint16_t said = addressConfig->getInt("said");
    coord.setAssociationDescription(said,description);
  }
  logger = Logger::getInstance("sctpmon");
  sender = new TrapSender();
  sender->Start();
  SctpMonitor *monitor = SctpMonitor::instance();
  monitor->addAssociationChangeListener(&coord);
  coord.setSctpMonitor(monitor);
  sigset( SIGTERM, sighandler );
#if 0
  int i = 0;
  while(true)
  {
    AssociationChangeEvent event;
    time(&event.time);
    event.said = i % 5;
    event.state = i % 10;
    coord.associationChange(event);
    i++;
    sleep(10);
  }
#endif
  monitor->run();
  sender->Stop();
}
