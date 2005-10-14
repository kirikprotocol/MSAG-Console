#ifndef _SCAG_ADMIN_SCAGCommand_H
#define	_SCAG_ADMIN_SCAGCommand_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"

#include "admin/service/Variant.h"
#include "admin/protocol/ResponseWriter.h"
#include "CommandActions.h"
#include "scag/scag.h"

#include "CommandIds.h"

namespace scag {
namespace admin {

using namespace smsc::admin::protocol;
using namespace scag;

class SCAGCommand : public Command
{
public:
  SCAGCommand(Command::Id id);
  virtual ~SCAGCommand();
  virtual Response * CreateResponse(scag::Scag * SmscApp);
  virtual scag::admin::Actions::CommandActions GetActions();
protected:
    smsc::logger::Logger *logger;
};

//===================================================================
//================= Sme commands ====================================

class Abstract_CommandSmeInfo : public scag::admin::SCAGCommand
{
public:
	Abstract_CommandSmeInfo(const Command::Id id, const xercesc::DOMDocument * const document);
	virtual ~Abstract_CommandSmeInfo();
  
  const scag::transport::smpp::SmppEntityInfo & getSmppEntityInfo() const {return smppEntityInfo;};
  
protected:
  scag::transport::smpp::SmppEntityInfo smppEntityInfo;
};

class CommandAddSme : public Abstract_CommandSmeInfo 
{
public:
  CommandAddSme(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmeInfo((Command::Id)CommandIds::addSme, document)
  {
  }
  virtual Response * CreateResponse(scag::Scag * SmscApp);

};

class CommandDeleteSme : public SCAGCommand
{
public:
  CommandDeleteSme(const xercesc::DOMDocument * const document);
  virtual Response * CreateResponse(scag::Scag * SmscApp);
protected:
    std::string systemId;

};

class CommandStatusSme : public Abstract_CommandSmeInfo 
{
public:
	CommandStatusSme(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmeInfo((Command::Id)CommandIds::statusSme, document)
  {
  }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandUpdateSmeInfo : public Abstract_CommandSmeInfo
{
public:
	CommandUpdateSmeInfo(const xercesc::DOMDocument * const document);
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

//========================================================================
//================== Smsc commands =======================================
class Abstract_CommandSmscInfo : public scag::admin::SCAGCommand
{
public:
	Abstract_CommandSmscInfo(const Command::Id id, const xercesc::DOMDocument * const document);
	virtual ~Abstract_CommandSmscInfo();
  
  const scag::transport::smpp::SmppEntityInfo & getSmppEntityInfo() const {return smppEntityInfo;};
  
protected:
  scag::transport::smpp::SmppEntityInfo smppEntityInfo;
};

class CommandUpdateSmsc : public Abstract_CommandSmscInfo 
{
public:
	CommandUpdateSmsc(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmscInfo((Command::Id)CommandIds::updateSmsc, document)
  {
  }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandAddSmsc : public Abstract_CommandSmscInfo 
{
public:
	CommandAddSmsc(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmscInfo((Command::Id)CommandIds::addSmsc, document)
  {
  }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

class CommandDeleteSmsc : public Abstract_CommandSmeInfo 
{
public:
	CommandDeleteSmsc(const xercesc::DOMDocument * const document)
    : Abstract_CommandSmeInfo ((Command::Id)CommandIds::deleteSmsc, document)
  {
  }
    virtual Response * CreateResponse(scag::Scag * SmscApp);
};

//========================================================================
//==================== TraceRoute command ================================
class CommandTraceRoute : public scag::admin::SCAGCommand 
{
public:
  
  CommandTraceRoute(const xercesc::DOMDocument * doc);
  virtual ~CommandTraceRoute();
  virtual Response * CreateResponse(scag::Scag * SmscApp);

protected:
  std::string dstAddr;
  std::string srcAddr;
  std::string srcSysId;
};

class CommandLoadRoutes : public scag::admin::SCAGCommand 
{
public:
  
  CommandLoadRoutes(const xercesc::DOMDocument * document);
  virtual ~CommandLoadRoutes();
  virtual Response * CreateResponse(scag::Scag * SmscApp);
};

//========================================================================
//==================== Rule commands =====================================
class CommandAddRule : public scag::admin::SCAGCommand
{
public:
  
  CommandAddRule(const xercesc::DOMDocument * doc);
  virtual ~CommandAddRule();
  virtual Response * CommandCreate(scag::Scag * SmscApp);

protected:
    int ruleId;
  
};

class CommandUpdateRule : public scag::admin::SCAGCommand
{
public:
  
  CommandUpdateRule(const xercesc::DOMDocument * doc);
  virtual ~CommandUpdateRule();
  virtual Response * CommandCreate(scag::Scag * SmscApp);

protected:
    int ruleId;
  
};

class CommandRemoveRule : public scag::admin::SCAGCommand
{
public:
  
  CommandRemoveRule(const xercesc::DOMDocument * doc);
  virtual ~CommandRemoveRule();
  virtual Response * CommandCreate(scag::Scag * SmscApp);

protected:
    int ruleId;
  
};

//========================================================================
//==================== Apply command =====================================
class CommandApply : public scag::admin::SCAGCommand 
{
public:
  enum subjects {
    unknown,
    config,
    routes,
    providers,
    smscs
  };
  
  CommandApply(const xercesc::DOMDocument * document);
  virtual ~CommandApply();
  virtual Response * CreateResponse(scag::Scag * ScagApp);
  virtual scag::admin::Actions::CommandActions GetActions();
  
  //subjects getSubject();

private:
  subjects subj;
};

}
}

#endif	/* _SCAG_ADMIN_SCAGCommand_H */

