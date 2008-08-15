#ifndef __SCAG_RULE_ENGINE_ACTION_RETURN2__
#define __SCAG_RULE_ENGINE_ACTION_RETURN2__

#include "scag/re/base/ActionContext2.h"
#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionReturn : public Action
{
    std::string m_sResultValue;
    int m_nResultValue;
    bool m_bResultExist;
    FieldType m_ftResult;

    StatusEnum m_bStatusValue;

    ActionReturn(const ActionReturn &);

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);

    ActionReturn (){};

    virtual ~ActionReturn();
};

}}}

#endif
