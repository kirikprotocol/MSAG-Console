#include "BillActionKeywords.h"
#include "scag/sessions/base/Session2.h"

namespace scag2 {
namespace re {
namespace actions {

const char* KEYWORDS_NAME = "keywords";

void BillActionSetKeywords::init( const SectionParams& params, PropertyObject propertyObject ) {
  BillAction::init(params, propertyObject);
  bool keywordsExists = false;
  keywordsType_ = CheckParameter(params, propertyObject, opname(), KEYWORDS_NAME, true, true, keywords_, keywordsExists);
  smsc_log_debug(logger, "Action '%s' init...", opname() );
}

bool BillActionSetKeywords::changeKeywords(ActionContext &context, Keywords< ActionContext, ExternalBillingTransaction >* keywords) {
  smsc_log_debug(logger, "Run Action '%s'...", opname());
  std::string transId = getTransId( context );
  if ( transId.empty() ) {
      smsc_log_error( logger, "Action '%s' cannot get transaction name", opname() );
      setBillingStatus( context, "cannot get transaction name", false );
      return true;
  }

  ExternalTransaction* etrans = context.getSession().getTransaction( transId.c_str() );
  if ( ! etrans ) {
      smsc_log_error( logger, "Action '%s' transaction '%s' not found",
                      opname(), transId.c_str() );
      setBillingStatus( context, "transaction not found", false );
      return true;
  }

  ExternalBillingTransaction* trans = etrans->castToBilling();
  if ( ! trans ) {
      smsc_log_error( logger, "Action '%s' transaction '%s' is not a billing one",
                      opname(), transId.c_str() );
      setBillingStatus( context, "not a billing transaction", false );
      return true;
  }
  if (keywords->change(trans)) {
    setBillingStatus(context, "", true);
  } else {
    setBillingStatus( context, "can't set keywords", false );
  }
  return true;
}

bool BillActionSetKeywords::run(ActionContext &context) {
  std::auto_ptr< SetKeywordsType > keywords(new SetKeywordsType(keywords_, keywordsType_ == ftUnknown, logger, context));
  return changeKeywords(context, keywords.get());
}

bool BillActionAddKeywords::run(ActionContext &context) {
  std::auto_ptr< AddKeywordsType > keywords(new AddKeywordsType(keywords_, keywordsType_ == ftUnknown, logger, context));
  return changeKeywords(context, keywords.get());
}

}
}
}
