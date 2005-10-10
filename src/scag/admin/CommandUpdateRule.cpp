#include "CommandUpdateRule.h"
#include "scag/re/RuleEngine.h"
#include "admin/service/Variant.h"
#include <xercesc/dom/DOM.hpp>
#include "util/xml/utilFunctions.h"

namespace scag {
namespace admin {

class RuleEngineException : public Exception
{
};

using smsc::admin::service::Variant;
using namespace xercesc;
using namespace smsc::util::xml;

CommandUpdateRule::CommandUpdateRule(const xercesc::DOMDocument * document)
    : SCAGCommand((Command::Id)CommandIds::updateRule),
      ruleId( -1 )
{
    smsc_log_info(logger, "CommandUpdateRule got parameters:");

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
      smsc_log_warn(logger, "Failed to read parameters of Update Rule command.");
    throw AdminException("Failed to read parameters of Update Rule command.");
  }
}

CommandUpdateRule::~CommandUpdateRule()
{
}

Response * CommandUpdateRule::CommandCreate(scag::Scag * ScagApp)
{
  smsc_log_info(logger, "CommandUpdateRule is processing...");
  try {
      scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
      re.updateRule(ruleId);

      smsc_log_info(logger, "CommandUpdateRule is processed ok.");
      return new Response(Response::Ok, "Failed to update rule. Unknown error.");

  }catch(RuleEngineException& e){

      char desc[512];
      sprintf(desc, "Failed to update rule. RuleEngineException exception, %s", e.what());
      Variant res((const char *)desc);

      smsc_log_info(logger, desc);
      return new Response(Response::Error, res);

  }catch(...){

      Variant res("Failed to update rule. Unknown error.");

      smsc_log_error(logger, "Failed to update rule. Unknown error.");
      return new Response(Response::Error, res);

  }

  smsc_log_error(logger, "Failed to update rule. Unknown error.");
  return new Response(Response::Error, "Failed to update rule. Unknown error.");
}

}
}