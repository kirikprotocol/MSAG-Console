#include "StatAction.h"
#include "scag/util/properties/Properties.h"
  
namespace scag2 { namespace re { namespace actions {

using scag::util::properties::Property;

const char* KEYWORDS_PARAM_NAME = "keywords";

void StatAction::initKeywordsParameter(const SectionParams &params, PropertyObject propertyObject, bool readOnly) {
  bool keywordsExists = false;
  keywordsType_ = CheckParameter(params, propertyObject, actionName(), KEYWORDS_PARAM_NAME, true, readOnly, keywords_, keywordsExists);
}

bool StatAction::getKeywords(ActionContext &context, string& keywords) {
  if (keywordsType_ == ftUnknown) {
    keywords = keywords_;
    return true;
  }
  Property *p = context.getProperty(keywords_);
  if(!p) {
    smsc_log_error(logger, "Action '%s': Invalid %s property: '%s'", actionName(), KEYWORDS_PARAM_NAME, keywords_.c_str());
    return false;
  }
  keywords = p->getStr();
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

void StatAction::setKeywords(const vector<string>& separated, Operation* op) {
  if (!op || separated.empty()) {
    return;
  }
  vector<string>::const_iterator i = separated.begin();
  string keywords = *i;
  ++i;
  for (i; i != separated.end(); ++i) {
    keywords += ',' + *i;
  }
  smsc_log_debug(logger, "Action '%s': result keywords='%s'", actionName(), keywords.c_str());
  op->setKeywords(keywords);
}

void AddKeywordsAction::init(const SectionParams &params, PropertyObject propertyObject) {
  initKeywordsParameter(params, propertyObject, true);  
}

bool AddKeywordsAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", actionName());
  Operation *op = context.getSession().getCurrentOperation();
  if (!op) {
    smsc_log_warn(logger, "Action '%s': there is no operation", actionName());
    return false;
  }
  string addKeywords;
  if (!getKeywords(context, addKeywords)) {
    return false;
  }
  vector<string> addSeparated;
  separateKeywords(addKeywords, addSeparated);
  if (addSeparated.empty()) {
    smsc_log_debug(logger, "Action '%s': no keywords for adding", actionName());
    return true;
  }
  const string* keywords = op->getKeywords();
  if (!keywords) {
    setKeywords(addSeparated, op);
    return true;
  }
  vector<string> separated;
  separateKeywords(*keywords, separated);
  if (separated.empty()) {
    setKeywords(addSeparated, op);
    return true;
  }
  vector<string> newKeywords;
  for (vector<string>::const_iterator i = addSeparated.begin(); i != addSeparated.end(); ++i) {
    if (find(separated.begin(), separated.end(), *i) == separated.end()) {
      newKeywords.push_back(*i);
    }
  }
  if (newKeywords.empty()) {
    return true;
  }
  separated.insert(separated.end(), newKeywords.begin(), newKeywords.end());
  setKeywords(separated, op);
  return true;
}

void SetKeywordsAction::init(const SectionParams &params, PropertyObject propertyObject) {
  initKeywordsParameter(params, propertyObject, true);  
}

bool SetKeywordsAction::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'", actionName());
  Operation *op = context.getSession().getCurrentOperation();
  if (!op) {
    smsc_log_warn(logger, "Action '%s': there is no operation", actionName());
    return false;
  }
  string keywords;
  if (!getKeywords(context, keywords)) {
    return false;
  }
  if (keywords.empty()) {
    op->setKeywords("");
    return true;
  }
  vector<string> separated;
  separateKeywords(keywords, separated);
  if (separated.empty()) {
    smsc_log_debug(logger, "Action '%s': error keywords '%s'", actionName(), keywords.c_str());
    return true;
  }
  setKeywords(separated, op);
  return true;
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
