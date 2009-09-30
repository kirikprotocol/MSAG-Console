#ifndef SCAG_RE_ACTIONS_IMPL_BILLACTIONTARIFF_H
#define SCAG_RE_ACTIONS_IMPL_BILLACTIONTARIFF_H

#include "BillAction.h"
#include "scag/re/base/IntField.h"
#include "scag/re/base/StringField.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionTariff : public BillAction
{
public:
    BillActionTariff();
    virtual const char* opname() const { return "bill:tariff"; }

protected:
    virtual void init( const SectionParams& params,
                       PropertyObject propertyObject );
    virtual bool run( ActionContext& context );

private:
    IntField    category_;
    StringField categoryStr_;
    IntField    contentType_;
    StringField contentTypeStr_;
    int         catId_;
    int         contId_;
    BaseField*  catField_;
    BaseField*  contField_;

    // --- output
    StringField resultServiceNumber_;
    StringField resultCurrency_;
    IntField    resultMediaType_;
    IntField    resultCategory_;
    IntField    resultBillType_;
    StringField resultPrice_;
};

}
}
}

#endif
