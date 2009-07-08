#ifndef __SCAG_RULE_ENGINE_ACTION_CLOSE_USSD_DIALOG2__
#define __SCAG_RULE_ENGINE_ACTION_CLOSE_USSD_DIALOG2__

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionCloseUssdDialog : public Action
{
    ActionCloseUssdDialog(const ActionCloseUssdDialog&);
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual const char* opname() const { return "smpp::close_ussd_dialog"; }
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionCloseUssdDialog() {}
};

}}}


#endif
