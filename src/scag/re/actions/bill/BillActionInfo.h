#ifndef _BILL_ACTION_INFO_
#define _BILL_ACTION_INFO_

#include "scag/re/actions/Action.h"

namespace scag { namespace re { namespace actions {

class BillActionInfo : public Action
{
    std::string m_sBillId;
    uint32_t m_BillId;

    static const int fields_count = 10;
    static const char* m_name[fields_count];
    std::string m_sField[fields_count];
    bool m_exist[fields_count], bExist;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params, const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    BillActionInfo() {};
    virtual bool run(ActionContext& context);
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};

}}}

#endif
