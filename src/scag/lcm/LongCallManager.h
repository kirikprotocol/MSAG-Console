/* $Id$ */

#ifndef SCAG_LCM_CLIENT_H
#define SCAG_LCM_CLIENT_H

#include "logger/Logger.h"
#include "scag/config/lcm/LongCallManagerConfig.h"
#include <stack>
#include "scag/util/SerializeBuffer.h"


namespace scag { namespace lcm {

using smsc::logger::Logger;
using namespace scag::config;
using smsc::core::synchronization;

enum LongCallCommandId{
    PERS_GET = 1,
    PERS_SET,
    PERS_DEL,
    PERS_INC,
    PERS_INC_MOD    
};

class LongCallInitiator;
struct LongCallContext;

struct ActionStackValue
{
    int actionIndex;
    bool thenSection;
    ActionStackValue(int index, bool flag) : actionIndex(index), thenSection(flag) {}
};



struct LongCallContext
{
    LongCallContext(LongCallInitiator* initr, uint32_t cid, void* p): initiator(initr), callCommandId(cid), param(p), next(NULL) {};
    
    uint32_t systemType, callCommandId;
    void *param;    
    LongCallInitiator *initiator;
    LongCallContext *next;

    std::stack<ActionStackValue> ActionStack;
    scag::util::SerializeBuffer contextActionBuffer;
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

