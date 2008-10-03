#ifndef _SCAG_RE_ACTIONS_BILL_BILLACTIONKEYWORDS_H
#define _SCAG_RE_ACTIONS_BILL_BILLACTIONKEYWORDS_H

#include "scag/re/actions/Action.h"

namespace scag {
namespace re {
namespace actions {

using std::string;

class BillActionKeywords : public Action {
public:
  BillActionKeywords():keywordsType_(ftUnknown) {};
  virtual ~BillActionKeywords() {};
  virtual void init(const SectionParams &params, PropertyObject propertyObject);
  virtual bool run(ActionContext &context);
  virtual bool FinishXMLSubSection(const std::string &name);
  virtual IParserHandler * StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory);

protected:
  virtual const char* opname() const = 0;
  void SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK);

protected:
  string keywords_;
  FieldType keywordsType_;
  std::string m_sBillId;
  uint32_t m_BillId;
  bool bExist;

private:
  string m_sStatus;
  string m_sMessage;
  bool m_StatusExist;
  bool m_MsgExist;
};

class BillActionSetKeywords : public BillActionKeywords {
public:
  BillActionSetKeywords() {};
  virtual bool run(ActionContext &context);

protected:
  virtual const char * opname() const { return "bill:set_keywords"; }
};

class BillActionAddKeywords : public BillActionKeywords {
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
