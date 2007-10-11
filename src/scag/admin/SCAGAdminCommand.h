#ifndef _SCAG_ADMIN_SCAGAdminCommand_H
#define _SCAG_ADMIN_SCAGAdminCommand_H

#include "AdminCommand.h"

//#include "scag/transport/SCAGCommand.h"

namespace scag {
namespace admin {

using namespace smsc::admin::protocol;
using namespace scag;




//===================================================================
//================= Sme commands ====================================

class Abstract_CommandSmeInfo : public AdminCommand
{
public:
    Abstract_CommandSmeInfo(Command::Id id, const xercesc::DOMDocument * const doc) : AdminCommand(id, doc)
    {
    }

    virtual void init();
    const scag::transport::smpp::SmppEntityInfo & getSmppEntityInfo() const {return smppEntityInfo;};
protected:
  scag::transport::smpp::SmppEntityInfo smppEntityInfo;
};

class CommandAddSme : public Abstract_CommandSmeInfo 
{
public:
  CommandAddSme(const xercesc::DOMDocument * const doc)
    : Abstract_CommandSmeInfo((Command::Id)CommandIds::addSme, doc)
  {
  }

  virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandDeleteSme : public AdminCommand
{
public:
  CommandDeleteSme(const xercesc::DOMDocument * const doc) 
    : AdminCommand((Command::Id)CommandIds::deleteSme, doc)
  {

  }

  virtual Response * CreateResponse(scag::Scag * SmscApp);
  virtual void init();
protected:
    std::string systemId;
    int disconn;

};

class CommandStatusSme : public Abstract_CommandSmeInfo 
{
public:
    CommandStatusSme(const xercesc::DOMDocument * const doc)
    : Abstract_CommandSmeInfo((Command::Id)CommandIds::statusSme, doc)
  {
  }

    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandUpdateSmeInfo : public Abstract_CommandSmeInfo
{
public:
    CommandUpdateSmeInfo(const xercesc::DOMDocument * const doc) :
        Abstract_CommandSmeInfo((Command::Id)CommandIds::updateSmeInfo, doc)
    {
    }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

//========================================================================
//==================== TraceSmppRoute command ================================
class CommandTraceSmppRoute : public AdminCommand 
{
public:
  CommandTraceSmppRoute(const xercesc::DOMDocument * doc) : AdminCommand((Command::Id)CommandIds::traceSmppRoute, doc)
  {
  }
  
  virtual Response * CreateResponse(scag::Scag * SmscApp);
  virtual void init();


protected:
  std::string dstAddr;
  std::string srcAddr;
  std::string srcSysId;
};

class CommandLoadSmppTraceRoutes : public AdminCommand 
{
public:
  
  CommandLoadSmppTraceRoutes(const xercesc::DOMDocument * doc) : AdminCommand((Command::Id)CommandIds::loadSmppTraceRoutes, doc)
  {
  }

  virtual Response * CreateResponse(scag::Scag * SmscApp);
};


//========================================================================
//==================== CommandApplyConfig command =====================================
class CommandApplyConfig : public AdminCommand 
{
public:
  
  CommandApplyConfig(const xercesc::DOMDocument * doc) 
    : AdminCommand((Command::Id)CommandIds::applyConfig, doc)
  {
  }

  virtual Response * CreateResponse(scag::Scag * ScagApp);
};


//========================================================================
//==================== CommandApplySmppRoutes command =====================================
class CommandApplySmppRoutes : public AdminCommand 
{
public:
  
  CommandApplySmppRoutes(const xercesc::DOMDocument * doc) 
    : AdminCommand((Command::Id)CommandIds::applySmppRoutes, doc)
  {
  }

  virtual Response * CreateResponse(scag::Scag * ScagApp);
};

//==================== CommandApplySmppRoutes command =====================================
class CommandApplyHttpRoutes : public AdminCommand 
{
public:
  
  CommandApplyHttpRoutes(const xercesc::DOMDocument * doc) 
    : AdminCommand((Command::Id)CommandIds::applyHttpRoutes, doc)
  {
  }

  virtual Response * CreateResponse(scag::Scag * ScagApp);
};


//------------------------ Bill infrastructure commands ------------------------
class CommandReloadOperators : public AdminCommand
{
public:
    CommandReloadOperators(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::reloadOperators, doc)
    {
    }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandReloadServices : public AdminCommand
{
public:
    CommandReloadServices(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::reloadServices, doc)
    {
    }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandReloadTariffMatrix : public AdminCommand
{
public:
    CommandReloadTariffMatrix(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::reloadTariffMatrix, doc)
    {
    }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandLoadHttpTraceRoutes : public AdminCommand
{
public:
    CommandLoadHttpTraceRoutes(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::loadHttpTraceRoutes, doc)
    {
    }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandTraceHttpRoute : public AdminCommand
{
    std::string addr, site, path;
    uint32_t port, sid, rid;
public:
    CommandTraceHttpRoute(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::traceHttpRoute, doc)
    {
    }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
    virtual void init();
};

class CommandGetLogCategories : public AdminCommand
{
public:
    CommandGetLogCategories(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::getLogCategories, doc)
    {
    }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandSetLogCategories : public AdminCommand
{
    std::string cats;
public:
    CommandSetLogCategories(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::setLogCategories, doc)
    {
    }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
    virtual void init();    
};

class CommandStoreLogConfig : public AdminCommand
{
public:
    CommandStoreLogConfig(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::storeLogConfig, doc)
    {
    }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};


class CommandListSmppEntity : public AdminCommand 
{
protected:
   virtual scag::transport::smpp::SmppEntityType getEntityType() = 0;
   virtual const char * getCommandName() = 0;
   virtual void fillResultSet(Variant& result, scag::transport::smpp::SmppEntityAdminInfoList& lst) = 0;
   std::string getStrBindType(scag::transport::smpp::SmppBindType bindType)
   {
       //btNone,btReceiver,btTransmitter,btRecvAndTrans,btTransceiver
       std::string str;

       if (bindType == scag::transport::smpp::btReceiver) str = "RX";
       else if (bindType == scag::transport::smpp::btTransmitter) str = "TX";
       else if (bindType == scag::transport::smpp::btRecvAndTrans) str = "RTX";
       else if (bindType == scag::transport::smpp::btTransceiver) str = "TRX";

       return str;
   }
public:
  CommandListSmppEntity(Command::Id commandId, const xercesc::DOMDocument * doc) : AdminCommand(commandId, doc) 
  {   
  }
 
  virtual Response * CreateResponse(scag::Scag * SmscApp);

};

class CommandListSme : public CommandListSmppEntity
{
protected:
    std::string name;

    virtual scag::transport::smpp::SmppEntityType getEntityType() {
        return scag::transport::smpp::etService;
    }

    const char * getCommandName() {
        return name.c_str();
    }

    virtual void fillResultSet(Variant& result, scag::transport::smpp::SmppEntityAdminInfoList& lst)
    {
        char buff[2048];
        for (scag::transport::smpp::SmppEntityAdminInfoList::iterator it = lst.begin(); it!=lst.end(); ++it) 
        {
            //if (it->host.size() == 0) 
            //    sprintf(buff, "SystemId, %s, Host, %s %s, Status, %s", it->systemId.c_str(), getStrBindType(it->bindType).c_str(), "Unknown", (it->connected) ? "yes" : "no");
            //else
            sprintf(buff, "SystemId, %s, Host, %s %s, Status, %s", it->systemId.c_str(), getStrBindType(it->bindType).c_str(), it->host.c_str(), (it->connected) ? "yes" : "no");

            result.appendValueToStringList(buff);
            smsc_log_debug(logger, "Command %s returns: %s", getCommandName(), buff);
        }
    }

public:
    CommandListSme(const xercesc::DOMDocument * doc) : name("CommandListSme"), 
        CommandListSmppEntity((Command::Id)CommandIds::listSme, doc) {   }

};

class CommandListSmsc : public CommandListSmppEntity
{
protected:
    std::string name;

    virtual scag::transport::smpp::SmppEntityType getEntityType() {
        return scag::transport::smpp::etSmsc;
    }

    virtual const char * getCommandName() {
        return name.c_str();
    }

    virtual void fillResultSet(Variant& result, scag::transport::smpp::SmppEntityAdminInfoList& lst)
    {
        char buff[2048];
        for (scag::transport::smpp::SmppEntityAdminInfoList::iterator it = lst.begin(); it!=lst.end(); ++it) 
        {
            //if (it->host.size() == 0) 
            //    sprintf(buff, "SystemId, %s, Host, %s %s, Port, %s, Status, %s", it->systemId.c_str(), getStrBindType(it->bindType).c_str(), "Unknown", "Unknown", (it->connected) ? "yes" : "no");
            //else
            sprintf(buff, "SystemId, %s, Host, %s %s, Port, %d, Status, %s", it->systemId.c_str(), getStrBindType(it->bindType).c_str(), it->host.c_str(), it->port, (it->connected) ? "yes" : "no");


            result.appendValueToStringList(buff);
            smsc_log_debug(logger, "Command %s returns: %s", getCommandName(), buff);
        }
    }
public:
    CommandListSmsc(const xercesc::DOMDocument * doc) : name("CommandListSmsc"),
        CommandListSmppEntity((Command::Id)CommandIds::listSmsc, doc) {   }
};



}
}

#endif  /* _SCAG_ADMIN_SCAGCommand_H */

