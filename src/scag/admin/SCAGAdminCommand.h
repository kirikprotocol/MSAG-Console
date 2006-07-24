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
    uint32_t port;
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

}
}

#endif  /* _SCAG_ADMIN_SCAGCommand_H */

