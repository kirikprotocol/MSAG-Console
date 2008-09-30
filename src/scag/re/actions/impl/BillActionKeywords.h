#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONKEYWORDS_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONKEYWORDS_H

#include "BillAction.h"

namespace scag2 {
namespace re {
namespace actions {

using std::string;

class BillActionSetKeywords : public BillAction {
public:
  BillActionSetKeywords():keywordsType_(ftUnknown) {};
  virtual ~BillActionSetKeywords() {};
  virtual bool run(ActionContext &context);

protected:
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual const char * opname() const { return "bill:set_keywords"; }

private:
  string keywords_;
  FieldType keywordsType_;
};

class BillActionAddKeywords : public BillActionSetKeywords {
public:
  BillActionAddKeywords() {};
  virtual bool run(ActionContext &context);

protected:
  virtual const char * opname() const { return "bill:add_keywords"; }
};

}
}
}

#endif /* !_SCAG_RE_ACTIONS_IMPL_BILLACTIONKEYWORDS_H */
