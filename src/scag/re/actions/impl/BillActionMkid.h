#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONMKID_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONMKID_H

#include "scag/re/base/Action2.h"
#include "scag/re/base/StringField.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionMkid : public Action
{
public:
    BillActionMkid() : id_(*this,"externalId",true,false) {}

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
    StringField id_;
    // FieldType   idtype_;
    // std::string idname_;
};

}}}

#endif
