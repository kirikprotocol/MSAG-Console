#ifndef __SCAG_RE_RULESTATUS__
#define __SCAG_RE_RULESTATUS__

namespace scag { namespace re {

struct RuleStatus
{
    bool result;
    bool hasErrors;
    RuleStatus() : result(false),hasErrors(false) {};
};

}}

#endif

