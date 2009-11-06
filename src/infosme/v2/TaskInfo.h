#ifndef SMSC_INFOSME2_TASKINFO_H
#define SMSC_INFOSME2_TASKINFO_H

#include "DateTime.h"
#include "logger/Logger.h"

namespace smsc {
namespace util { namespace config { class ConfigView; }}
namespace infosme2 {

class Task;

struct TaskInfo
{
    friend class Task;

public:
    static unsigned stringToTaskId( const std::string& taskId );

    TaskInfo();
    void init( unsigned uid, smsc::util::config::ConfigView* config );

    inline bool isDelivery() const { return delivery; }

private:
    static smsc::logger::Logger* log_;

private:
    // unsigned    uid;
    std::string name;
    // std::string location;
    unsigned    priority;
    bool        enabled, delivery, retryOnFail, replaceIfPresent,
        trackIntegrity, transactionMode, keepHistory,
        saveFinalState, flash, useDataSm;

    time_t  endDate;            // full date/time
    time_t  validityPeriod;     // only HH:mm:ss in seconds
    time_t  validityDate;       // full date/time
    time_t  activePeriodStart;  // only HH:mm:ss in seconds
    time_t  activePeriodEnd;    // only HH:mm:ss in seconds

    std::string retryPolicy;    // only if retryOnFail is true

    WeekDaysSet activeWeekDays; // Mon, Tue ...

    std::string tablePrefix;
    std::string querySql;
    std::string msgTemplate;
    std::string svcType;        // specified if replaceIfPresent == true
    std::string address;

    int     dsTimeout, dsUncommitedInProcess, dsUncommitedInGeneration;
    int     messagesCacheSize, messagesCacheSleep;
};

}
}

#endif

