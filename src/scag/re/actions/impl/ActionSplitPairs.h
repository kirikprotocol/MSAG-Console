#ifndef __SCAG_RULE_ENGINE_ACTION_SPLITPAIRS__
#define __SCAG_RULE_ENGINE_ACTION_SPLITPAIRS__

#include "ActionPairsBase.h"


namespace scag2 {
namespace re {
namespace actions {

class ActionSplitPairs : public ActionPairsBase {
public:
    ActionSplitPairs():ActionPairsBase(true),
                       source_(*this, "source", true, true),
                       rest_(*this, "rest", false, false) {}
    ~ActionSplitPairs() {};
    void init(const SectionParams &params, PropertyObject propertyObject);
    bool run(ActionContext &context);
    const char* opname() const { return "strings:split_pairs"; }

private:
    StringField source_;
    StringField rest_;
};

}}}

#endif
