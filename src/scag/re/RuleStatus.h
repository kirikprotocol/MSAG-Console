#ifndef __SCAG_RE_RULESTATUS__
#define __SCAG_RE_RULESTATUS__

namespace scag { namespace re {

struct RuleStatus
{
    int result;
    bool status;
    bool temporal;
    //RuleStatus() : result(false),hasErrors(false) {};
    RuleStatus() : result(0), status(false),temporal(false) {};
};

}}

#endif

