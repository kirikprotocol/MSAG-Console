#ifndef _SCAG_COUNTER_AVERAGINGMANAGER_H
#define _SCAG_COUNTER_AVERAGINGMANAGER_H

#include <string>
#include "scag/util/MsecTime.h"

namespace scag2 {
namespace counter {

class AveragingGroup;

/// An item may be attached to the averaging group
class AverageItem 
{
public:
    virtual ~AverageItem() {}

    virtual void reset() = 0;
    virtual void average( util::MsecTime::time_type curtime ) = 0;

protected:
    /// NOTE: to be invoked from ctor/dtor of successor classes
    void addItem( AveragingGroup& grp );
    void remItem( AveragingGroup& grp );
};


class AveragingManager
{
public:
    virtual ~AveragingManager() {}
    virtual void addGroup( AveragingGroup& group, util::MsecTime::time_type wakeTime ) = 0;
    virtual void remGroup( AveragingGroup& group ) = 0;
};

}
}

#endif
