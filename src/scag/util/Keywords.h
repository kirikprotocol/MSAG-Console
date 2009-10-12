#ifndef _SCAG_UTIL_KEYWORDS_H_
#define _SCAG_UTIL_KEYWORDS_H_

#include <string>
#include <vector>
#include <algorithm>

#include "scag/util/properties/Properties.h"
#include "logger/Logger.h"

namespace scag {
namespace util { 

using std::string;
using std::vector;
using scag::util::properties::Property;
using smsc::logger::Logger;

template<class T, class K>
class Keywords {
public:  
  Keywords(const string& keywords, bool fieldTypeUnknown, Logger* log, T& actionContext)
           :value_(keywords), fieldTypeUnknown_(fieldTypeUnknown), actionContext_(actionContext), logger(log) { };
  virtual ~Keywords() {};
  virtual bool change(K* keywordsContext) {
    if (!keywordsContext) {
      smsc_log_error(logger, "Action '%s': keywords context not defined", actionName());
      return false;
    }
    keywordsContext_ = keywordsContext;
    if (fieldTypeUnknown_) {
      return true;
    }
      typename T::PropertyType *p = actionContext_.getProperty(value_);
    if(!p) {
      smsc_log_error(logger, "Action '%s': Invalid keywords property: '%s'", actionName(), value_.c_str());
      return false;
    }
    value_ = p->getStr().c_str();
    return true;
  }

protected:
  void separateKeywords(const string& keywords, vector<string>& separatedKeywords) {
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
  void setFormatedValue(const vector<string>& separatedKeywords) {
    if (separatedKeywords.empty()) {
      return;
    }
      size_t sz = separatedKeywords.size(); // for commas and trailing nullchar
      for ( vector<string>::const_iterator i = separatedKeywords.begin();
            i != separatedKeywords.end(); ++i ) {
          sz += i->size();
      }
      string keywords;
      keywords.reserve( sz );
      for ( vector<string>::const_iterator i = separatedKeywords.begin();
            i != separatedKeywords.end(); ++i ) {
          if ( i != separatedKeywords.begin() ) keywords.push_back(',');
          keywords.append(*i);
      }
    smsc_log_debug(logger, "Action '%s': result keywords='%s'", actionName(), keywords.c_str());
    keywordsContext_->setKeywords(keywords);

  }
  virtual const char* actionName() = 0;

protected:
  string value_;
  string setValue_;
  bool fieldTypeUnknown_;
  T& actionContext_;
  K* keywordsContext_;
  Logger* logger;
};

template<class T, class K>
class AddKeywords : public Keywords<T, K>{
public:
  AddKeywords(const string& keywords, bool fieldTypeUnknown, Logger* log, T& actionContext)
             :Keywords<T, K>(keywords, fieldTypeUnknown, log, actionContext) {};
  virtual bool change(K* keywordsContext) {
    if (!Keywords<T, K>::change(keywordsContext)) {
      return false;
    }
    vector<string> addSeparated;
    separateKeywords(this->value_, addSeparated);
    if (addSeparated.empty()) {
      smsc_log_debug(this->logger, "Action '%s': no keywords for adding", actionName());
      return true;
    }
    const string* keywords = this->keywordsContext_->getKeywords();
    if (!keywords) {
      this->setFormatedValue(addSeparated);
      return true;
    }
    vector<string> separated;
    this->separateKeywords(*keywords, separated);
    if (separated.empty()) {
      this->setFormatedValue(addSeparated);
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
    this->setFormatedValue(separated);
    return true;
  }
protected:
  const char* actionName() { return "add_keywords"; }
};

template<class T, class K>
class SetKeywords : public Keywords<T, K> {
public:
  SetKeywords(const string& keywords, bool fieldTypeUnknown, Logger* log, T& actionContext)
              :Keywords<T, K>(keywords, fieldTypeUnknown, log, actionContext) {};
  virtual bool change(K* keywordsContext) {
    if (!Keywords<T, K>::change(keywordsContext)) {
      return false;
    }
    if (this->value_.empty()) {
      this->keywordsContext_->setKeywords("");
      return true;
    }
    vector<string> separated;
    this->separateKeywords(this->value_, separated);
    if (separated.empty()) {
        smsc_log_debug(this->logger, "Action '%s': error keywords '%s'", actionName(), this->value_.c_str());
      return true;
    }
    this->setFormatedValue(separated);
    return true;
  }
protected:
  const char* actionName() { return "set_keywords"; }
};

}//util
}//scag

namespace scag2 {
namespace util {
using scag::util::Keywords;
using scag::util::AddKeywords;
using scag::util::SetKeywords;
}
}

#endif
