#include "StatAction.h"
#include "scag/util/properties/Properties.h"
  
namespace scag { namespace re { namespace actions {

using scag::util::properties::Property;

void StatAction::initKeywordsParameter(const SectionParams &params, PropertyObject propertyObject, bool readOnly) {
  bool keywordsExists = false;
  FieldType keywordsType = CheckParameter(params, propertyObject, actionName(), "keywords", true, readOnly, keywords_, keywordsExists);
  if (keywordsType != ftUnknown) {
    keywordsTypeUnknown_ = false;
  }
}  

bool StatAction::FinishXMLSubSection(const std::string &name) {
  return true;
}

IParserHandler * StatAction::StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory) {
  throw SCAGException("Action '%s': cannot have a child object", actionName());
}

void AddKeywordsAction::init(const SectionParams &params, PropertyObject propertyObject) {
  initKeywordsParameter(params, propertyObject, true);  
}

bool AddKeywordsAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", actionName());
  Operation *op = context.GetCurrentOperation();
  if (!op) {
    smsc_log_warn(logger, "Action '%s':  operation not found", actionName());
    return false;
  }

  std::auto_ptr< KeywordsAction > keywordsRuner(new KeywordsAction(keywords_, keywordsTypeUnknown_, logger, context));
  return keywordsRuner->run(op);
}

void SetKeywordsAction::init(const SectionParams &params, PropertyObject propertyObject) {
  initKeywordsParameter(params, propertyObject, true);  
}

bool SetKeywordsAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", actionName());
  Operation *op = context.GetCurrentOperation();
  if (!op) {
    smsc_log_warn(logger, "Action '%s': operation not found", actionName());
    return false;
  }

  std::auto_ptr< KeywordsAction > keywordsRuner(new KeywordsAction(keywords_, keywordsTypeUnknown_, logger, context));
  return keywordsRuner->run(op);
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
  Operation *op = context.getSession().GetCurrentOperation();
  if (!op) {
    smsc_log_warn(logger, "Action '%s':  operation not found", actionName());
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
}//scag

