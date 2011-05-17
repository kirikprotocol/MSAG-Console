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
#include "snmp/sctp/NullTrapSender.hpp"
#include "snmp/sctp/SnmpTrapSender.hpp"
#include <string>
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "util/findConfigFile.h"
#include "core/buffers/XHash.hpp"

using smsc::core::buffers::XHash;
using smsc::snmp::sctp::SctpMonitor;
using smsc::snmp::sctp::TrapSender;
using smsc::snmp::sctp::SnmpTrapSender;
using smsc::snmp::sctp::NullTrapSender;
using smsc::snmp::SnmpAgent;
using smsc::snmp::sctp::AssociationChangeListener;
using smsc::snmp::sctp::AssociationChangeEvent;
using smsc::snmp::sctp::getAssociationStateDescription;
using smsc::logger::Logger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;
using std::string;

struct Association {
  uint16_t said;
  uint8_t  state;
  string   descr;
};


static Logger* logger = 0;
class Coordinator;
static Coordinator *coord = 0;

class hash_func_said{
public:
  static inline unsigned CalcHash(uint16_t said){
    return (unsigned)said;
  }
};

class Coordinator: public AssociationChangeListener{
  private:
    SctpMonitor *monitor;
    TrapSender *sender;
    XHash<unsigned,Association*,hash_func_said> assocs;
  public:
    Coordinator() { monitor = 0; sender = 0; }
    void setSctpMonitor(SctpMonitor *_monitor) { monitor = _monitor; }
    void setTrapSender(TrapSender *_sender) { sender = _sender; }
    void start()
    {
      if ( monitor == 0 || sender == 0 )
      {
        smsc_log_debug(logger, "Signalling monitor or Trap sender is invalid. Exiting...");
        return;
      }
      monitor->addAssociationChangeListener(this);
      sender->run();
      monitor->run();
      sender->stop();
    }
    void stop() {
      monitor->removeAssociationChangeListener();
      monitor->stop();
    }
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
            severity = SnmpAgent::CRITICAL;
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
  if (coord) coord->stop();
}

int main (int argc, char *argv[])
{
  Logger::Init();
  logger = Logger::getInstance("sctpmon");
  coord = new Coordinator();
  if ( !coord )
  {
    smsc_log_debug(logger, "Association Coordinator has not been created. Exiting...");
    return -1;
  }
  smsc::util::config::Manager::init(findConfigFile("config.xml"));
  Manager& manager = Manager::getInstance();
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
    coord->setAssociationDescription(said,description);
  }
  static const char *trapSenderType = "snmp";
  try
  {
    trapSenderType = manager.getString("sctpmon.trapSenderType");
  } 
  catch (ConfigException& exc)
  { 
    smsc_log_debug(logger, "trapSenderType is not defined, \"snmp\" will be used");
    trapSenderType="snmp";
  }
  TrapSender *sender = 0;
  if (strcmp(trapSenderType,"snmp") == 0)
  { 
    sender = new SnmpTrapSender();
  }
  else if (strcmp(trapSenderType,"null") == 0)
  {
    sender = new NullTrapSender();
  }
  SctpMonitor *monitor = SctpMonitor::instance();
  coord->setSctpMonitor(monitor);
  coord->setTrapSender(sender);
  sigset( SIGTERM, sighandler );
  coord->start();
}
