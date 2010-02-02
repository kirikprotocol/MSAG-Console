#ifndef _SCAG_ADMIN_SCAGAdminCommand2_H
#define _SCAG_ADMIN_SCAGAdminCommand2_H

#include "AdminCommand2.h"

//#include "scag/transport/SCAGCommand.h"

namespace scag2 {
namespace admin {

using namespace smsc::admin::protocol;
// using namespace scag;


//===================================================================
//================= Sme commands ====================================

class Abstract_CommandSmeInfo : public AdminCommand
{
public:
    Abstract_CommandSmeInfo(Command::Id id, const xercesc::DOMDocument * const doc) : AdminCommand(id, doc)
    {
    }

    virtual void init();
protected:
  std::string systemId;
};

class CommandAddSme : public Abstract_CommandSmeInfo
{
public:
  CommandAddSme(const xercesc::DOMDocument * const doc)
    : Abstract_CommandSmeInfo((Command::Id)CommandIds::addSme, doc)
  {
  }

  virtual Response * CreateResponse( Scag * SmscApp);
};

class CommandDeleteSme : public AdminCommand
{
public:
  CommandDeleteSme(const xercesc::DOMDocument * const doc)
    : AdminCommand((Command::Id)CommandIds::deleteSme, doc)
  {

  }

  virtual Response * CreateResponse(Scag * SmscApp);
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

    virtual Response * CreateResponse(Scag * SmscApp);
};

class CommandUpdateSmeInfo : public Abstract_CommandSmeInfo
{
public:
    CommandUpdateSmeInfo(const xercesc::DOMDocument * const doc) :
        Abstract_CommandSmeInfo((Command::Id)CommandIds::updateSme, doc)
    {
    }
    virtual Response * CreateResponse(Scag * SmscApp);
};

//========================================================================
//==================== TraceSmppRoute command ================================
class CommandTraceSmppRoute : public AdminCommand
{
public:
  CommandTraceSmppRoute(const xercesc::DOMDocument * doc) : AdminCommand((Command::Id)CommandIds::traceSmppRoute, doc)
  {
  }

  virtual Response * CreateResponse(Scag * SmscApp);
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

  virtual Response * CreateResponse(Scag * SmscApp);
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

  virtual Response * CreateResponse(Scag * ScagApp);
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

  virtual Response * CreateResponse(Scag * ScagApp);
};

//==================== CommandApplySmppRoutes command =====================================
class CommandApplyHttpRoutes : public AdminCommand
{
public:

  CommandApplyHttpRoutes(const xercesc::DOMDocument * doc)
    : AdminCommand((Command::Id)CommandIds::applyHttpRoutes, doc)
  {
  }

  virtual Response * CreateResponse(Scag * ScagApp);
};


//------------------------ Bill infrastructure commands ------------------------
class CommandReloadOperators : public AdminCommand
{
public:
    CommandReloadOperators(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::reloadOperators, doc)
    {
    }
    virtual Response * CreateResponse(Scag * SmscApp);
};

class CommandReloadServices : public AdminCommand
{
public:
    CommandReloadServices(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::reloadServices, doc)
    {
    }
    virtual Response * CreateResponse(Scag * SmscApp);
};

class CommandReloadTariffMatrix : public AdminCommand
{
public:
    CommandReloadTariffMatrix(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::reloadTariffMatrix, doc)
    {
    }
    virtual Response * CreateResponse(Scag * SmscApp);
};

class CommandLoadHttpTraceRoutes : public AdminCommand
{
public:
    CommandLoadHttpTraceRoutes(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::loadHttpTraceRoutes, doc)
    {
    }
    virtual Response * CreateResponse(Scag * SmscApp);
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
    virtual Response * CreateResponse(Scag * SmscApp);
    virtual void init();
};

class CommandGetLogCategories : public AdminCommand
{
public:
    CommandGetLogCategories(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::getLogCategories, doc)
    {
    }
    virtual Response * CreateResponse(Scag * SmscApp);
};

class CommandSetLogCategories : public AdminCommand
{
    std::string cats;
public:
    CommandSetLogCategories(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::setLogCategories, doc)
    {
    }
    virtual Response * CreateResponse(Scag * SmscApp);
    virtual void init();
};

class CommandStoreLogConfig : public AdminCommand
{
public:
    CommandStoreLogConfig(const xercesc::DOMDocument * const doc) :
        AdminCommand((Command::Id)CommandIds::storeLogConfig, doc)
    {
    }
    virtual Response * CreateResponse(Scag * SmscApp);
};


class CommandListSmppEntity : public AdminCommand
{
protected:
    virtual transport::smpp::SmppEntityType getEntityType() = 0;
    virtual const char * getCommandName() = 0;
    virtual void fillResultSet(Variant& result, transport::smpp::SmppEntityAdminInfoList& lst) = 0;
    std::string getStrBindType(transport::smpp::SmppBindType bindType)
    {
        //btNone,btReceiver,btTransmitter,btRecvAndTrans,btTransceiver
        std::string str;

        if (bindType == transport::smpp::btReceiver) str = "RX";
        else if (bindType == transport::smpp::btTransmitter) str = "TX";
        else if (bindType == transport::smpp::btRecvAndTrans) str = "RTX";
        else if (bindType == transport::smpp::btTransceiver) str = "TRX";

        return str;
    }
public:
  CommandListSmppEntity(Command::Id commandId, const xercesc::DOMDocument * doc) : AdminCommand(commandId, doc)
  {
  }

  virtual Response * CreateResponse(Scag * SmscApp);

};

class CommandListSme : public CommandListSmppEntity
{
protected:
    std::string name;

    virtual transport::smpp::SmppEntityType getEntityType() {
        return transport::smpp::etService;
    }

    const char * getCommandName() {
        return name.c_str();
    }

    virtual void fillResultSet(Variant& result, transport::smpp::SmppEntityAdminInfoList& lst)
    {
        char buff[2048];
        for (transport::smpp::SmppEntityAdminInfoList::iterator it = lst.begin(); it!=lst.end(); ++it)
        {
            //if (it->host.size() == 0)
            //    snprintf(buff, sizeof(buff), "SystemId, %s, Host, %s %s, Status, %s", it->systemId.c_str(), getStrBindType(it->bindType).c_str(), "Unknown", (it->connected) ? "yes" : "no");
            //else
            snprintf(buff, sizeof(buff), "SystemId, %s, Host, %s %s, Status, %s", it->systemId.c_str(), getStrBindType(it->bindType).c_str(), it->host.c_str(), (it->connected) ? "yes" : "no");

            result.appendValueToStringList(buff);
            smsc_log_debug(logger, "Command %s returns: %s", getCommandName(), buff);
        }
    }

public:
    CommandListSme(const xercesc::DOMDocument * doc) :
    CommandListSmppEntity((Command::Id)CommandIds::listSme, doc), name("CommandListSme") {}

};

class CommandListSmsc : public CommandListSmppEntity
{
protected:
    std::string name;

    virtual transport::smpp::SmppEntityType getEntityType() {
        return transport::smpp::etSmsc;
    }

    virtual const char * getCommandName() {
        return name.c_str();
    }

    virtual void fillResultSet(Variant& result, transport::smpp::SmppEntityAdminInfoList& lst)
    {
        char buff[2048];
        for (transport::smpp::SmppEntityAdminInfoList::iterator it = lst.begin(); it!=lst.end(); ++it)
        {
            //if (it->host.size() == 0)
            //    snprintf(buff, sizeof(buff), "SystemId, %s, Host, %s %s, Port, %s, Status, %s", it->systemId.c_str(), getStrBindType(it->bindType).c_str(), "Unknown", "Unknown", (it->connected) ? "yes" : "no");
            //else
            snprintf(buff, sizeof(buff), "SystemId, %s, Host, %s %s, Port, %d, Status, %s", it->systemId.c_str(), getStrBindType(it->bindType).c_str(), it->host.c_str(), it->port, (it->connected) ? "yes" : "no");


            result.appendValueToStringList(buff);
            smsc_log_debug(logger, "Command %s returns: %s", getCommandName(), buff);
        }
    }
public:
    CommandListSmsc(const xercesc::DOMDocument * doc) :
    CommandListSmppEntity((Command::Id)CommandIds::listSmsc, doc),
    name("CommandListSmsc") {}
};

class CommandMetaEntity:public AdminCommand
{
public:
  CommandMetaEntity(Command::Id id,const xercesc::DOMDocument * doc):
    AdminCommand(id,doc)
  {

  }
  virtual void init();
protected:
  std::string systemId;
};

class CommandAddMetaEntity:public CommandMetaEntity{
public:
  CommandAddMetaEntity(const xercesc::DOMDocument * doc)
    : CommandMetaEntity((Command::Id)CommandIds::addMetaEntity, doc)
  {
  }
  virtual Response * CreateResponse(Scag *ScagApp);
};

class CommandUpdateMetaEntity:public CommandMetaEntity{
public:
  CommandUpdateMetaEntity(const xercesc::DOMDocument * doc)
    : CommandMetaEntity((Command::Id)CommandIds::addMetaEntity, doc)
  {
  }
  virtual Response * CreateResponse(Scag *ScagApp);
};

class CommandDeleteMetaEntity:public AdminCommand{
public:
  CommandDeleteMetaEntity(const xercesc::DOMDocument * doc)
    : AdminCommand((Command::Id)CommandIds::deleteMetaEntity, doc)
  {
  }
  virtual Response * CreateResponse(Scag *ScagApp);
  virtual void init();
protected:
  std::string systemId;
};

class CommandMetaEndpoint:public AdminCommand{
public:
  CommandMetaEndpoint(Command::Id id,const xercesc::DOMDocument * doc)
    : AdminCommand(id, doc)
  {
  }
  virtual void init();
protected:
  std::string metaId;
  std::string sysId;
};

class CommandAddMetaEndpoint:public CommandMetaEndpoint{
public:
  CommandAddMetaEndpoint(const xercesc::DOMDocument * doc)
    : CommandMetaEndpoint((Command::Id)CommandIds::addMetaEndpoint, doc)
  {
  }

  virtual Response * CreateResponse(Scag *ScagApp);
};

class CommandRemoveMetaEndpoint:public CommandMetaEndpoint{
public:
  CommandRemoveMetaEndpoint(const xercesc::DOMDocument * doc)
    : CommandMetaEndpoint((Command::Id)CommandIds::removeMetaEndpoint, doc)
  {
  }

  virtual Response * CreateResponse(Scag *ScagApp);
};


class CommandReplaceCounter : public AdminCommand
{
public:
    CommandReplaceCounter( CommandIds::IDS id, const xercesc::DOMDocument* doc ) :
    AdminCommand((Command::Id)id,doc) {}
    virtual void init();
    virtual Response* CreateResponse( Scag* ScagApp );
protected:
    std::string id;
};

}
}

#endif  /* _SCAG_ADMIN_SCAGCommand_H */

