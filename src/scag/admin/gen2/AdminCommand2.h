#ifndef _SCAG_ADMIN_AdminCommand2_H
#define	_SCAG_ADMIN_AdminCommand2_H

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "admin/service/Variant.h"
#include "admin/protocol/ResponseWriter.h"
#include "scag/admin/CommandActions.h"
#include "scag/admin/CommandIds.h"
#include "scag/gen2/scag2.h"

namespace scag2 {
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

class AdminCommand : public Command
{
public:
    static const char* responseEncoding;

public:
  AdminCommand(Command::Id id, const xercesc::DOMDocument * const doc) :
      Command(id), logger(smsc::logger::Logger::getInstance("admin.command"))
  {
      document = doc;
  }

  virtual ~AdminCommand(){}


  virtual Response * CreateResponse( Scag * ScagApp )
  {
      smsc_log_info(logger, "AdminCommand::CreateResponse has processed");
      return new Response(Response::Ok, "none", responseEncoding);
  }

/*  virtual Actions::CommandActions GetActions()
  {
      Actions::CommandActions result;
      return result;
  }*/

  virtual void init() {}

protected:
    smsc::logger::Logger *logger;
    const xercesc::DOMDocument * document;
};


}}

#endif	

