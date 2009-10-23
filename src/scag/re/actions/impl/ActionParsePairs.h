#ifndef __SCAG_RULE_ENGINE_ACTION_PARSEPAIRS__
#define __SCAG_RULE_ENGINE_ACTION_PARSEPAIRS__

#include "ActionPairsBase.h"


namespace scag2 {
namespace re {
namespace actions {

class ActionParsePairs : public ActionPairsBase {
public:
    ActionParsePairs():source_(*this, "source", true, true),
                       rest_(*this, "rest", false, false) {}
    ~ActionParsePairs() {};
    void init(const SectionParams &params, PropertyObject propertyObject);
    bool run(ActionContext &context);
    const char* opname() const { return "strings:parse_pairs"; }

private:
    StringField source_;
    StringField rest_;
};

}}}

#endif

