/* $Id$ */

#ifndef SCAG_LCM_CLIENT_H
#define SCAG_LCM_CLIENT_H

#include "logger/Logger.h"
#include "scag/config/lcm/LongCallManagerConfig.h"

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

struct LongCallContext
{
    LongCallContext(LongCallInitiator* initr, uint32_t cid, void* p): initiator(initr), callCommandId(cid), param(p), next(NULL) {};
    
    uint32_t systemType, callCommandId;
    void *param;    
    LongCallInitiator *initiator;
    LongCallContext *next;
};

class LongCallInitiator
{
public:
    virtual void continueExecution(LongCallContext* context) = 0;
};


class LongCallManager {
protected:
    virtual ~LongCallManager() {};

public:
    static LongCallManager& Instance();
    static void Init(uint32_t maxThr);
    static void Init(const LongCallManagerConfig& cfg);
    
    virtual void call(LongCallContext* context) = 0;

protected:
    static bool  inited;
    static Mutex initLock;
};

}}

#endif

