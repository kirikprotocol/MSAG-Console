#ifndef _SCAG_COUNTER_IMPL_NOTIFICATIONMANAGER_H
#define _SCAG_COUNTER_IMPL_NOTIFICATIONMANAGER_H

#include "logger/Logger.h"
#include "scag/counter/TimeSliceManager.h"

namespace scag2 {
namespace counter {
namespace impl {

class NotificationManager : public TimeSliceItem
{

public:
    NotificationManager();
    virtual ~NotificationManager();

    virtual void advanceTime( usec_type curTime );

protected:
    virtual TimeSliceGroup* getTimeSliceGroup() const { return 0; }
    virtual void doSetTimeSliceGroup( TimeSliceGroup* ) {}
    virtual usec_type getTimeSliceWidth() const { return 0; }

private:
    smsc::logger::Logger* log_;
};

}
}
}

#endif /* ! _SCAG_COUNTER_IMPL_NOTIFICATIONMANAGER_H */
