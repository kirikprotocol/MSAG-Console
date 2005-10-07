#include "CommandRemoveRule.h"
#include "scag/re/RuleEngine.h"
#include "admin/service/Variant.h"
#include <xercesc/dom/DOM.hpp>
#include "util/xml/utilFunctions.h"

namespace scag {
namespace admin {

using smsc::admin::service::Variant;
using namespace xercesc;
using namespace smsc::util::xml;

class RuleEngineException : public Exception
{
};

CommandRemoveRule::CommandRemoveRule(const xercesc::DOMDocument * document)
    : SCAGCommand((Command::Id)CommandIds::updateRule),
      ruleId( -1 )
{

  smsc_log_info(logger, "CommandRemoveRule got parameters:");

  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));
      
      
      if (::strcmp("ruleId", name) == 0){
        ruleId = atoi(value.get());
        smsc_log_info(logger, "ruleId: %d", ruleId);
      }

    }

  } catch (...) {
      smsc_log_warn(logger, "Failed to read parameters of remove rule command.");
    throw AdminException("Some exception occured");
  }
}

CommandRemoveRule::~CommandRemoveRule()
{
}

Response * CommandRemoveRule::CommandCreate(scag::Scag * SmscApp)
{
  smsc_log_info(logger, "CommandRemoveRule is processing...");
  try {
      scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
      re.updateRule(ruleId);

      smsc_log_info(logger, "CommandRemoveRule is processed ok");
      return new Response(Response::Ok, "none");

  }catch(RuleEngineException& e){

      char desc[512];
      sprintf(desc, "Failed to remove rule. RuleEngine exception: %s", e.what());
      smsc_log_error(logger, desc);
      Variant res((const char *)desc);

      return new Response(Response::Error, res);

  }catch(...){

      Variant res("Failed to remove rule. Unknown error");
      smsc_log_error(logger, "Failed to remove rule. Unknown error");
      return new Response(Response::Error, res);

  }

  smsc_log_error(logger, "Unknown error");
  return new Response(Response::Error, "Unknown error");
}

}
}