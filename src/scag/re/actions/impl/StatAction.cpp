#include "StatAction.h"
#include "scag/util/properties/Properties.h"
  
namespace scag2 { namespace re { namespace actions {

using scag::util::properties::Property;

const char* KEYWORDS_PARAM_NAME = "keywords";

void StatAction::init(const SectionParams &params, PropertyObject propertyObject) {
  bool keywordsExists = false;
  keywordsType_ = CheckParameter(params, propertyObject, actionName(), KEYWORDS_PARAM_NAME, true, true, keywords_, keywordsExists);
}

bool StatAction::run(ActionContext &context) {
  if (keywordsType_ == ftUnknown) {
    return true;
  }
  Property *p = context.getProperty(keywords_);
  if(!p) {
    smsc_log_error(logger, "Action '%s': Invalid %s property: '%s'", actionName(), KEYWORDS_PARAM_NAME, keywords_.c_str());
    return false;
  }
  keywords_ = p->getStr();
  return true;
}

bool StatAction::FinishXMLSubSection(const std::string &name) {
  return true;
}

IParserHandler * StatAction::StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory) {
  throw SCAGException("Action '%s': cannot have a child object", actionName());
}

void StatAction::separateKeywords(const string& keywords, vector<string>& separatedKeywords) {
  //smsc_log_debug(logger, "keywords='%s'", keywords.c_str());
  if (keywords.empty()) {
    return;
  }
  size_t start_pos = keywords.find_first_not_of(" \t\n,");
  if (start_pos == string::npos) {
    return;
  }
  size_t del_pos = 0;
  do {
    del_pos = keywords.find_first_of(',', start_pos);
    size_t end_pos = del_pos == string::npos ? keywords.length() - 1 : del_pos - 1;
    while (isspace(keywords[end_pos])) { --end_pos; }
    size_t length = end_pos - start_pos + 1;
    if (length > 0) {
      string keyword(keywords, start_pos, length);
      separatedKeywords.push_back(keyword);
      //smsc_log_debug(logger, "keyword='%s'", keyword.c_str());
    }
    start_pos = keywords.find_first_not_of(" \t\n,", del_pos);
  } while (del_pos != string::npos && start_pos != string::npos);
}

bool AddKeywordsAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", actionName());
  if (!StatAction::run(context)) {
    return false;
  }
  smsc_log_debug(logger, "Action '%s': keywords='%s'", actionName(), keywords_.c_str());
  vector<string> separated;
  separateKeywords(keywords_, separated);
  //TODO:
  //context.keywords += keywords_;
  return true;
}

bool SetKeywordsAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", actionName());
  if (!StatAction::run(context)) {
    return false;
  }
  smsc_log_debug(logger, "Action '%s': keywords='%s'", actionName(), keywords_.c_str());
  //TODO:
  //context.keywords = keywords_;
  return true;
}

void GetKeywordsAction::init(const SectionParams &params, PropertyObject propertyObject) {
  bool keywordsExists = false;
  keywordsType_ = CheckParameter(params, propertyObject, actionName(), KEYWORDS_PARAM_NAME, true, false, keywords_, keywordsExists);
}

bool GetKeywordsAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", actionName());
  Property *p = context.getProperty(keywords_);
  if (p) {
    smsc_log_debug(logger, "Action '%s': keywords='%s'", actionName(), keywords_.c_str());
    //TODO:
    p->setStr("keywords");
  }
  return true;
}

}//actions
}//re
}//scag2
