#ifndef __SCAG_RE_RULESTATUS__
#define __SCAG_RE_RULESTATUS__
#include "logger/Logger.h"

namespace scag { namespace re {

typedef enum StatusEnum
{
    STATUS_OK,
    STATUS_FAILED,
    STATUS_REDIRECT,
    STATUS_LONG_CALL,
    STATUS_PROCESS_LATER
} StatusEnum;

struct RuleStatus
{
    int result;
    StatusEnum status;
    bool temporal;
    //RuleStatus() : result(false),hasErrors(false) {};
    RuleStatus() : result(0), status(STATUS_OK),temporal(false) {};
    ~RuleStatus() {}
private:
    RuleStatus(const RuleStatus& cp) {};
    RuleStatus& operator=(const RuleStatus& cp) { return *this; };
};

}}

#endif
