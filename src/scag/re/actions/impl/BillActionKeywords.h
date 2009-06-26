#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONKEYWORDS_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONKEYWORDS_H

#include "BillAction.h"
#include "scag/util/Keywords.h"
#include "scag/sessions/base/ExternalBillingTransaction.h"


namespace scag2 {
namespace re {
namespace actions {

using std::string;
using util::Keywords;
using util::AddKeywords;
using util::SetKeywords;
using sessions::ExternalBillingTransaction;

class BillActionSetKeywords : public BillAction {
public:
  BillActionSetKeywords() : BillAction(false), keywordsType_(ftUnknown) {};
  virtual ~BillActionSetKeywords() {};
  virtual bool run(ActionContext &context);

protected:
  bool changeKeywords(ActionContext &context, Keywords< ActionContext, ExternalBillingTransaction >* keywords);
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual const char * opname() const { return "bill:set_keywords"; }

protected:
  string keywords_;
  FieldType keywordsType_;

private:
  typedef SetKeywords< ActionContext, ExternalBillingTransaction > SetKeywordsType;
};

class BillActionAddKeywords : public BillActionSetKeywords {
public:
  BillActionAddKeywords() {};
  virtual bool run(ActionContext &context);

protected:
  virtual const char * opname() const { return "bill:add_keywords"; }

private:
  typedef AddKeywords< ActionContext, ExternalBillingTransaction > AddKeywordsType;
};

}
}
}

#endif /* !_SCAG_RE_ACTIONS_IMPL_BILLACTIONKEYWORDS_H */
