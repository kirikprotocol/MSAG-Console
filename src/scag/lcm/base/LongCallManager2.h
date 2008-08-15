/* $Id$ */

#ifndef SCAG_LCM_CLIENT2_H
#define SCAG_LCM_CLIENT2_H

#include "scag/re/base/LongCallContext.h"

namespace scag2 {
namespace lcm {

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


class LongCallManager 
{

public:
    static LongCallManager& Instance();

    virtual ~LongCallManager();
    virtual bool call(LongCallContext* context) = 0;
    virtual void shutdown() = 0;

protected:
    LongCallManager();
};

}}

#endif
