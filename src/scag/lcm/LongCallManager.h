/* $Id$ */

#ifndef SCAG_LCM_CLIENT_H
#define SCAG_LCM_CLIENT_H

#include "logger/Logger.h"
#include "scag/config/lcm/LongCallManagerConfig.h"
#include <stack>
#include <sms/sms_serializer.h>

namespace scag { namespace lcm {

using smsc::logger::Logger;
using namespace scag::config;
using smsc::core::synchronization;
using namespace smsc::core::buffers;

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

class LongCallBuffer : public smsc::sms::BufOps::SmsBuffer
{
protected:
    LongCallBuffer& operator >> (std::string& str)
    {
        uint8_t len;
        this->Read((char*)&len,1);
        char scb[256];
    
        if (len>255) throw smsc::util::Exception("Attempt to read %d byte in buffer with size %d",(int)len,255);
    
        this->Read(scb,len);
        scb[len] = 0;
    
        str = scb;
        return *this;
    };
public:
    LongCallBuffer() : smsc::sms::BufOps::SmsBuffer(2048) {}
    LongCallBuffer(int size):smsc::sms::BufOps::SmsBuffer(size){}
};


struct LongCallContext
{
    LongCallContext(LongCallInitiator* initr, uint32_t cid, void* p): initiator(initr), callCommandId(cid), param(p), next(NULL) {};
    
    uint32_t systemType, callCommandId;
    void *param;    
    LongCallInitiator *initiator;
    LongCallContext *next;

    std::stack<ActionStackValue> ActionStack;
    LongCallBuffer contextActionBuffer;
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

