#include "BillActionKeywords.h"
#include "scag/sessions/base/ExternalBillingTransaction.h"
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

bool BillActionSetKeywords::run(ActionContext &context) {
  smsc_log_debug(logger, "Run Action '%s'...", opname());
  std::string transId = getTransId( context );
  if ( transId.empty() ) {
      // smsc_log_error( logger, "Action '%s' cannot get transaction name", opname() );
      // setBillingStatus( context, "cannot get transaction name", false );
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
  setBillingStatus(context, "", true);
  //TODO: set keywords to billing transaction
  return true;
}

bool BillActionAddKeywords::run(ActionContext &context) {
  BillActionSetKeywords::run(context);
  //TODO: add keywords to billing transaction
  return true;
}

}
}
}
