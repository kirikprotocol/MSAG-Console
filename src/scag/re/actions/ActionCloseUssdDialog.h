#ifndef __SCAG_RULE_ENGINE_ACTION_CLOSE_USSD_DIALOG__
#define __SCAG_RULE_ENGINE_ACTION_CLOSE_USSD_DIALOG__

#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionCloseUssdDialog : public Action
{
    ActionCloseUssdDialog(const ActionCloseUssdDialog&);
    Logger * logger;
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionCloseUssdDialog() {}
};

}}}


#endif
