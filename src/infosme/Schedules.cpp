
#include "Schedules.h"

namespace smsc { namespace infosme 
{

time_t OnceSchedule::calulateNextTime()
{
    return -1;
}
time_t DailySchedule::calulateNextTime()
{
    return -1;
}
time_t WeeklySchedule::calulateNextTime()
{
    return -1;
}
time_t MonthlySchedule::calulateNextTime()
{
    return -1;
}

}}
