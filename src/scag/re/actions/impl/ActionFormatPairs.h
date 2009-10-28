#ifndef __SCAG_RULE_ENGINE_ACTION_FORMATPAIRS__
#define __SCAG_RULE_ENGINE_ACTION_FORMATPAIRS__

#include "ActionPairsBase.h"
#include "ActionPair.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionFormatPairs : public ActionPairsBase {
public:
    ActionFormatPairs():ActionPairsBase(true, true),
                       prefix_(*this, "prefix", false, true),
                       postfix_(*this, "postfix", false, true),
                       result_(*this, "result", true, false) {}
    ~ActionFormatPairs() {};
    virtual void init(const SectionParams &params, PropertyObject propertyObject);
    virtual bool run(ActionContext &context);
    virtual const char* opname() const { return "strings:format_pairs"; }

private:
    StringField prefix_;
    StringField postfix_;
    StringField result_;
};

}}}

#endif

