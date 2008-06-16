/* $Id$ */

#ifndef SCAG_LCM_CLIENT_H
#define SCAG_LCM_CLIENT_H

#include "logger/Logger.h"
#include "scag/config/lcm/LongCallManagerConfig.h"
//#include "scag/re/actions/ActionContext.h"

#include <stack>

namespace scag { namespace re { namespace actions {
    class ActionContext;
}}}

namespace scag { namespace lcm {

using smsc::logger::Logger;
using namespace scag::config;
using namespace smsc::core::synchronization;
using scag::re::actions::ActionContext;

enum LongCallCommandId{
    PERS_GET = 1,
    PERS_SET,
    PERS_DEL,
    PERS_INC,
    PERS_INC_MOD,
  PERS_BATCH,
    BILL_OPEN,
    BILL_COMMIT,
    BILL_ROLLBACK
};

class LongCallParams{
public:
    std::string exception;
    virtual ~LongCallParams() {};
};

class LongCallInitiator;
struct LongCallContext;

struct ActionStackValue
{
    int actionIndex;
    bool thenSection;
    ActionStackValue(int index, bool flag) : actionIndex(index), thenSection(flag) {}
};

class PostProcessAction
{
public:
    PostProcessAction* next;
    PostProcessAction() : next(NULL) {};
    virtual void run() = 0;
    virtual ~PostProcessAction() {};
};

class LongCallContext
{
    LongCallParams *params;
    ActionContext  *actionContext;
    PostProcessAction *actions, *actionsTail;
public:
    LongCallContext(): initiator(NULL), stateMachineContext(NULL),
  next(NULL), params(NULL), actionContext(NULL), continueExec(false), actions(NULL) {};

    uint32_t systemType, callCommandId;
    void *stateMachineContext;
    LongCallInitiator *initiator;
    LongCallContext *next;
    bool continueExec;

    std::stack<ActionStackValue> ActionStack;

    LongCallParams* getParams() { return params; };
    void setParams(LongCallParams* p)
    {
        if(params) delete params;
       params = p;
    }
    void freeParams()
    {
        if(params) delete params;
        params = NULL;
    }

    void setActionContext(ActionContext* context);
    ActionContext* getActionContext();

    void addAction(PostProcessAction* p)
    {
        if(actions)
            actionsTail->next = p;
        else
            actions = p;
        actionsTail = p;
    }
    void runPostProcessActions()
    {
        PostProcessAction* p;
        while(actions)
        {
            actions->run();
            p = actions->next;
            delete actions;
            actions=p;
        }
        actionsTail=actions;
    }

    void clear();

    ~LongCallContext();
};

class LongCallInitiator
{
public:
    virtual void continueExecution(LongCallContext* context, bool dropped) = 0;
};


class LongCallManager {
protected:
    virtual ~LongCallManager() {};

public:
    static LongCallManager& Instance();
    static void Init(uint32_t maxThr);
    static void Init(const LongCallManagerConfig& cfg);
    static void shutdown();

    virtual bool call(LongCallContext* context) = 0;

protected:
    static bool  inited;
    static Mutex initLock;
};

}}

#endif
