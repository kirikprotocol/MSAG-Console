#include "CommandUpdateRule.h"
#include "scag/re/RuleEngine.h"
#include "admin/service/Variant.h"
#include <xercesc/dom/DOM.hpp>
#include "util/xml/utilFunctions.h"

namespace scag {
namespace admin {

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
      smsc_log_info(logger, "CommandUpdateRule exception, unknown exception");
    throw AdminException("Some exception occured");
  }
}

CommandUpdateRule::~CommandUpdateRule()
{
}

Response * CommandUpdateRule::CommandCreate(scag::Scag * SmscApp)
{
  smsc_log_info(logger, "CommandUpdateRule is processing...");
  try {
      scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
      re.updateRule(ruleId);

  }catch(Exception e){

      char desc[512];
      sprintf(desc, "RuleEngine exception: %s", e.what());
      Variant res((const char *)desc);

      smsc_log_info(logger, "CommandUpdateRule exception, %s", e.what());
      return new Response(Response::Error, res);

  }catch(...){

      Variant res("Unknown exception");

      smsc_log_info(logger, "CommandUpdateRule exception, unknown exception");
      return new Response(Response::Error, res);

  }

  smsc_log_info(logger, "CommandUpdateRule is processed ok");
  return new Response(Response::Ok, "none");
}

}
}