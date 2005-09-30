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

CommandRemoveRule::CommandRemoveRule(const xercesc::DOMDocument * document)
    : SCAGCommand((Command::Id)CommandIds::updateRule),
      ruleId( -1 )
{
  try {
    DOMElement *elem = document->getDocumentElement();
    DOMNodeList *list = elem->getElementsByTagName(XmlStr("param"));
    for (int i=0; i<list->getLength(); i++) {
      DOMElement *paramElem = (DOMElement*) list->item(i);
      XmlStr name(paramElem->getAttribute(XmlStr("name")));
      std::auto_ptr<char> value(getNodeText(*paramElem));
      
      
      if (::strcmp("ruleId", name) == 0)
        ruleId = atoi(value.get());

    }

  } catch (...) {
    throw AdminException("Some exception occured");
  }
}

CommandRemoveRule::~CommandRemoveRule()
{
}

Response * CommandRemoveRule::CommandCreate(scag::Scag * SmscApp)
{
  try {
      scag::re::RuleEngine& re = scag::re::RuleEngine::Instance();
      re.updateRule(ruleId);

  }catch(Exception e){

      char desc[512];
      sprintf(desc, "RuleEngine exception: %s", e.what());
      Variant res((const char *)desc);

      return new Response(Response::Error, res);

  }catch(...){

      Variant res("Unknown exception");
      return new Response(Response::Error, res);

  }

  return new Response(Response::Ok, "none");
}

}
}