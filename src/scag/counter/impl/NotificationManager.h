#ifndef _SCAG_COUNTER_IMPL_NOTIFICATIONMANAGER_H
#define _SCAG_COUNTER_IMPL_NOTIFICATIONMANAGER_H

#include "logger/Logger.h"
#include "scag/counter/TimeSliceManager.h"
#include "scag/counter/Counter.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/FixedLengthString.hpp"

namespace scag2 {
namespace counter {
namespace impl {

class NotificationManager : public TimeSliceItem
{

public:
    NotificationManager();
    virtual ~NotificationManager();

    virtual void advanceTime( usec_type curTime );
    void notify( const char* cname, CntSeverity& sev,
                 int64_t value, const ActionLimit& limit );

protected:
    virtual TimeSliceGroup* getTimeSliceGroup() const { return 0; }
    virtual void doSetTimeSliceGroup( TimeSliceGroup* ) {}
    virtual usec_type getTimeSliceWidth() const { return 0; }

private:
    void initMapping();
    struct TrapData;
    TrapData* findTrapData( const char* cname, const char** idFound = 0 );

private:
    struct SendData {
        SendData() {}
        SendData(int64_t v, CntSeverity s, unsigned ser) :
        value(v), severity(s), serial(ser) {}
        int64_t               value;     // current value
        CntSeverity           severity;  // current severity
        unsigned              serial;
    };

    struct TrapData {
        TrapData() {}
        TrapData(int trap0, const char* cat0 = 0) : trap(trap0), cat(cat0) {}
        int                                        trap;
        smsc::core::buffers::FixedLengthString<32> cat;
    };

    // typedef std::list< CounterPtr > CounterList;
    typedef smsc::core::buffers::Hash<SendData> SendDataHash;
    typedef smsc::core::buffers::Hash<TrapData> MappingHash;

    smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    MappingHash                        mappingHash_;
    SendDataHash                       sendDataHash_;
    unsigned                           serial_;
};

}
}
}

#endif /* ! _SCAG_COUNTER_IMPL_NOTIFICATIONMANAGER_H */
