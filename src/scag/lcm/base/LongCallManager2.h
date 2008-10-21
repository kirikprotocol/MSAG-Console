/* $Id$ */

#ifndef SCAG_LCM_CLIENT2_H
#define SCAG_LCM_CLIENT2_H

#include "scag/re/base/LongCallContextBase.h"

namespace scag2 {
namespace lcm {

class LongCallManager 
{

public:
    static LongCallManager& Instance();

    virtual ~LongCallManager();
    virtual bool call(LongCallContextBase* context) = 0;
    virtual void shutdown() = 0;

protected:
    LongCallManager();
};

}}

#endif
