#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONMKID_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONMKID_H

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionMkid : public Action
{
public:
    BillActionMkid() {}

protected:
    // Action iface
    virtual void init( const SectionParams& params,
                       PropertyObject propertyObject );
    virtual IParserHandler* StartXMLSubSection(const std::string& name, const SectionParams& params, const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
    virtual bool run( ActionContext& context );

    virtual const char* opname() const {
        return "bill:mkid";
    }

private:
    FieldType   idtype_;
    std::string idname_;
};

}}}

#endif
