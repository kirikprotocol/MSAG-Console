#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONKEYWORDS_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONKEYWORDS_H

#include "BillAction.h"
#include "scag/util/Keywords.h"
#include "scag/sessions/base/ExternalBillingTransaction.h"


namespace scag2 {
namespace re {
namespace actions {

using std::string;
using scag::util::AddKeywords;
using scag::util::SetKeywords;
using scag2::sessions::ExternalBillingTransaction;

class BillActionSetKeywords : public BillAction {
public:
  BillActionSetKeywords():keywordsType_(ftUnknown) {};
  virtual ~BillActionSetKeywords() {};
  virtual bool run(ActionContext &context);

protected:
  ExternalBillingTransaction* getBillTransaction(ActionContext &context);
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual const char * opname() const { return "bill:set_keywords"; }

protected:
  string keywords_;
  FieldType keywordsType_;

private:
  typedef SetKeywords< ActionContext, ExternalBillingTransaction > KeywordsAction;
};

class BillActionAddKeywords : public BillActionSetKeywords {
public:
  BillActionAddKeywords() {};
  virtual bool run(ActionContext &context);

protected:
  virtual const char * opname() const { return "bill:add_keywords"; }

private:
  typedef AddKeywords< ActionContext, ExternalBillingTransaction > KeywordsAction;
};

}
}
}

#endif /* !_SCAG_RE_ACTIONS_IMPL_BILLACTIONKEYWORDS_H */
