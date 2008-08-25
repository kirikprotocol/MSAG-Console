#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTION_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTION_H

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class BillAction : public Action
{
protected:
    // action iface
    virtual void init( const SectionParams& params,
                       PropertyObject propertyObject );
    virtual IParserHandler* StartXMLSubSection( const std::string& name,
                                                const SectionParams& params,
                                                const ActionFactory& factory );
    virtual bool FinishXMLSubSection(const std::string& name);



    std::string getTransId( ActionContext& ctx );

    void setBillingStatus( ActionContext& context,
                           const char* errmsg,
                           bool isok );

    virtual const char* opname() const = 0;

private:
    // -- input
    FieldType       transIdFieldType_;
    std::string     transIdFieldName_;

    // -- output

    // FieldType       statusFieldType_;
    std::string     statusFieldName_;
    bool            hasStatus_;

    // FieldType       msgFieldType_;
    std::string     msgFieldName_;
    bool            hasMessage_;

};

}
}
}

#endif /* !_SCAG_RE_ACTIONS_IMPL_BILLACTION_H */
