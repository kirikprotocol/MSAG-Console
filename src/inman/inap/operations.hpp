#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_OPERATIONS__
#define __SMSC_INMAN_INAP_OPERATIONS__

#include "ss7cp.h"

namespace smsc {
namespace inman {
namespace inap {

class Operation
{
    protected:
        OPER_T op;
    public:
        virtual COMP_T* get() const = 0;
};

namespace op
{

/*
ACTIVITYTESTARG_T activityTestArg;
CAMELAPPLYCHARGINGARG_T capApplyChargingArg;
CAMELAPPLYCHARGINGREPORTARG_TcapApplyChargingReport-
Arg;
CANCELARG_T cancelArg;
CONNECTARG_T connectArg;
CONTINUEARG_T continueArg;
EVENTREPORTBCSMARG_T eventReportBCSMArg;
INITIALDPARG_T initialDPArg;
RELEASECALLARG_T releaseCallArg;
REQUESTREPORTBCSMEVENTARG_T requestReportBCSMEventArg;
*/

class InitialDP : public Operation
{
    public:
    COMP_T* get() const;
};

class RequestReportBCSMEvent : public Operation
{
    public:
    COMP_T* get() const;
};

class EventReportBCSM : public Operation
{
    public:
    COMP_T* get() const;
};

class Cancel : public Operation
{
    public:
    COMP_T* get() const;
};

class Connect : public Operation
{
    public:
    COMP_T* get() const;
};

class Continue : public Operation
{
    public:
    COMP_T* get() const;
};

class ReleaseCall : public Operation
{
    public:
    COMP_T* get() const;
};

class ApplyCharging : public Operation
{
    public:
    COMP_T* get() const;
};

class ApplyChargingReport : public Operation
{
    public:
    COMP_T* get() const;
};

}
}
}
}

#endif
