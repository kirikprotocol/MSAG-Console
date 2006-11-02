#ifndef __SCAG_RE_RULESTATUS__
#define __SCAG_RE_RULESTATUS__

namespace scag { namespace re {

enum StatusEnum
{
    STATUS_OK,
    STATUS_FAILED,
    STATUS_REDIRECT
};

struct RuleStatus
{
    int result;
    StatusEnum status;
    bool temporal;
    //RuleStatus() : result(false),hasErrors(false) {};
    RuleStatus() : result(0), status(STATUS_FAILED),temporal(false) {};
};

}}

#endif

