#ifndef _STAT_ACTION_H_
#define _STAT_ACTION_H_

#include <string>
#include <vector>

#include "scag/re/base/Action2.h"

namespace scag2 { namespace re { namespace actions {

using std::string;
using std::vector;

class StatAction : public Action {
public:
  StatAction():keywordsType_(ftUnknown) {};
  ~StatAction() {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual bool run(ActionContext &context);
  virtual bool FinishXMLSubSection(const std::string &name);
  virtual IParserHandler * StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory);

protected:
  void separateKeywords(const string& keywords, vector<string>& separatedKeywords);
  virtual const char* actionName() const = 0;

protected:
  string keywords_;
  FieldType keywordsType_;

};

class AddKeywordsAction : public StatAction {
public:
  AddKeywordsAction() {};
  virtual bool run(ActionContext &context);

protected:
  const char* actionName() const { return "stat:add_keywords"; }
};

class SetKeywordsAction : public StatAction {
public:
  SetKeywordsAction() {};
  virtual bool run(ActionContext &context);

protected:
  const char* actionName() const { return "stat:set_keywords"; }
};

class GetKeywordsAction : public StatAction {
public:
  GetKeywordsAction() {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual bool run(ActionContext &context);

protected:
  const char* actionName() const { return "stat:get_keywords"; }
};

}//actions
}//re
}//scag2

#endif
