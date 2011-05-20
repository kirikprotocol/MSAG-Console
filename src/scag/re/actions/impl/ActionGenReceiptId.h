#ifndef _SCAG_RE_ACTIONS_IMPL_ACTIONGENRECEIPTID_H
#define _SCAG_RE_ACTIONS_IMPL_ACTIONGENRECEIPTID_H

#include "scag/re/base/Action2.h"
#include "scag/re/base/StringField.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionGenReceiptId : public Action
{
public:
    ActionGenReceiptId() : id_(*this,"var",true,false) {}

protected:
    // Action iface
    virtual void init( const SectionParams& params,
                       PropertyObject propertyObject );
    virtual IParserHandler* StartXMLSubSection(const std::string& name,
                                               const SectionParams& params,
                                               const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
    virtual bool run( ActionContext& context );

    virtual const char* opname() const {
        return "smpp:gen_receipt_id";
    }

private:
    StringField id_;
};

}}}

#endif
