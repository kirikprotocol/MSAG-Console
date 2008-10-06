#include "StatAction.h"
#include "scag/util/properties/Properties.h"
#include "scag/util/Keywords.h"
  
namespace scag2 { namespace re { namespace actions {

using scag::util::properties::Property;

//const char* KEYWORDS_PARAM_NAME = "keywords";

void StatAction::initKeywordsParameter(const SectionParams &params, PropertyObject propertyObject, bool readOnly) {
  bool keywordsExists = false;
  keywordsType_ = CheckParameter(params, propertyObject, actionName(), "keywords", true, readOnly, keywords_, keywordsExists);
}

bool StatAction::FinishXMLSubSection(const std::string &name) {
  return true;
}

IParserHandler * StatAction::StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory) {
  throw SCAGException("Action '%s': cannot have a child object", actionName());
}

bool StatAction::changeKeywords(ActionContext &context, Keywords< ActionContext, Operation >* keywords) {
  smsc_log_debug(logger, "Run Action '%s'", actionName());
  Operation *op = context.getSession().getCurrentOperation();
  if (!op) {
    smsc_log_warn(logger, "Action '%s': there is no operation", actionName());
    return false;
  }
  return keywords->change(op);
}

void AddKeywordsAction::init(const SectionParams &params, PropertyObject propertyObject) {
  initKeywordsParameter(params, propertyObject, true);  
}

bool AddKeywordsAction::run(ActionContext &context) {
  std::auto_ptr< AddKeywordsType > keywords(new AddKeywordsType(keywords_, keywordsType_ == ftUnknown, logger, context));
  return changeKeywords(context, keywords.get());
}

void SetKeywordsAction::init(const SectionParams &params, PropertyObject propertyObject) {
  initKeywordsParameter(params, propertyObject, true);  
}

bool SetKeywordsAction::run(ActionContext &context) {
  std::auto_ptr< SetKeywordsType > keywords(new SetKeywordsType(keywords_, keywordsType_ == ftUnknown, logger, context));
  return changeKeywords(context, keywords.get());
}

void GetKeywordsAction::init(const SectionParams &params, PropertyObject propertyObject) {
  initKeywordsParameter(params, propertyObject, false);  
}

bool GetKeywordsAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", actionName());
  Property *p = context.getProperty(keywords_);
  if (!p) {
    smsc_log_warn(logger, "Action '%s': invalid property '%s'", actionName(), keywords_.c_str());
    return false;
  }
  Operation *op = context.getSession().getCurrentOperation();
  if (!op) {
    smsc_log_warn(logger, "Action '%s': there is no operation", actionName());
    return false;
  }
  const string* keywords = op->getKeywords();
  if (keywords) {
    p->setStr(*keywords);
  } else {
    p->setStr("");
  }
  return true;
}

}//actions
}//re
}//scag2
