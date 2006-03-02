#ifndef _SCAG_ADMIN_SCAGCommand_H
#define	_SCAG_ADMIN_SCAGCommand_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"

#include "admin/service/Variant.h"
#include "admin/protocol/ResponseWriter.h"
#include "CommandActions.h"

#include "scag/scag.h"
#include "scag/transport/SCAGCommand.h"


#include "CommandIds.h"

namespace scag {
namespace admin {

#define GETSTRPARAM(param, name_)                           \
    if (::strcmp(name_, name) == 0){                        \
        strcpy(param, value.get());                         \
        smsc_log_info(logger, name_ ": %s", param);         \
    }                                                   

#define GETSTRPARAM_(param, name_)                          \
    if (::strcmp(name_, name) == 0){                        \
        param = value.get();                                \
        smsc_log_info(logger, name_ ": %s", param.c_str()); \
    }                                                   

#define GETINTPARAM(param, name_)                           \
    if (::strcmp(name_, name) == 0){                        \
        param = atoi(value.get());                          \
        smsc_log_info(logger, name_ ": %d", param);         \
    }

#define BEGIN_SCAN_PARAMS                                               \
    DOMElement *elem = document->getDocumentElement();                  \
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));    \
    for (int i=0; i<list->getLength(); i++) {                           \
      DOMElement *paramElem = (DOMElement*) list->item(i);              \
      XmlStr name(paramElem->getAttribute(XmlStr("name")));             \
      std::auto_ptr<char> value(getNodeText(*paramElem));
      
#define END_SCAN_PARAMS }



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

class CommandDeleteSmsc : public SCAGCommand
{    
protected:
    std::string systemId;
public:
	CommandDeleteSmsc(const xercesc::DOMDocument * const document);
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

