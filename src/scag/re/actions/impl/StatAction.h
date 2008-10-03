#ifndef _STAT_ACTION_H_
#define _STAT_ACTION_H_

#include <string>
#include <vector>

#include "scag/re/base/Action2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/util/Keywords.h"

namespace scag2 { namespace re { namespace actions {

using std::string;
using std::vector;
using scag2::sessions::Session;
using scag2::sessions::Operation;
using scag::util::AddKeywords;
using scag::util::SetKeywords;

class StatAction : public Action {
public:
  StatAction():keywordsType_(ftUnknown) {}; 
  ~StatAction() {};
  virtual bool FinishXMLSubSection(const std::string &name);
  virtual IParserHandler * StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory);

protected:
  void initKeywordsParameter(const SectionParams &params, PropertyObject propertyObject, bool readOnly);
  virtual const char* actionName() const = 0;

protected:
  string keywords_;
  FieldType keywordsType_;
};

class AddKeywordsAction : public StatAction {
public:
  AddKeywordsAction() {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual bool run(ActionContext &context);

protected:
  typedef AddKeywords< ActionContext, Operation > KeywordsAction;
  const char* actionName() const { return "stat:add_keywords"; }
};

class SetKeywordsAction : public StatAction {
public:
  SetKeywordsAction() {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual bool run(ActionContext &context);

protected:
  typedef SetKeywords< ActionContext, Operation > KeywordsAction;
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
