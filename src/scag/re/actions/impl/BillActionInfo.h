#ifndef _SCAG_RE_ACTIONS_IMPL_BILLACTIONINFO_H
#define _SCAG_RE_ACTIONS_IMPL_BILLACTIONINFO_H

#include "BillAction.h"

namespace scag2 {
namespace re {
namespace actions {

class BillActionInfo : public BillAction
{
public:
    BillActionInfo() {}

protected:

    // Action iface
    virtual void init( const SectionParams& params,
                       PropertyObject propertyObject );
    virtual bool run( ActionContext& context );
    // virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params, const ActionFactory& factory);
    // virtual bool FinishXMLSubSection(const std::string& name);

    virtual const char* opname() const {
        return "bill:info";
    }

private:
    static const int fields_count = 12;
    static const char* m_name[fields_count];

private:
    std::string fieldName_[fields_count];
    bool hasField_[fields_count];

};

}}}

#endif
