#ifndef _SCAG_ADMIN_SCAGAdminCommand_H
#define	_SCAG_ADMIN_SCAGAdminCommand_H

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
//==================== TraceRoute command ================================
class CommandTraceRoute : public AdminCommand 
{
public:
  CommandTraceRoute(const xercesc::DOMDocument * doc) : AdminCommand((Command::Id)CommandIds::traceRoute, doc)
  {
  }
  
  virtual Response * CreateResponse(scag::Scag * SmscApp);
  virtual void init();


protected:
  std::string dstAddr;
  std::string srcAddr;
  std::string srcSysId;
};

class CommandLoadRoutes : public AdminCommand 
{
public:
  
  CommandLoadRoutes(const xercesc::DOMDocument * doc) : AdminCommand((Command::Id)CommandIds::loadRoutes, doc)
  {
  }

  virtual Response * CreateResponse(scag::Scag * SmscApp);
};

//========================================================================
//==================== Apply command =====================================
class CommandApply : public AdminCommand 
{
public:
  enum subjects {
    unknown,
    config,
    routes,
    providers,
    smscs
  };
  
  CommandApply(const xercesc::DOMDocument * doc) 
    : AdminCommand((Command::Id)CommandIds::apply, doc)
  {
  }

  virtual Response * CreateResponse(scag::Scag * ScagApp);
  virtual scag::admin::Actions::CommandActions GetActions();
  virtual void init();

private:
  subjects subj;
};

}
}

#endif	/* _SCAG_ADMIN_SCAGCommand_H */

