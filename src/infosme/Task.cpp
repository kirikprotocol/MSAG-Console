
#include "Task.h"

namespace smsc { namespace infosme 
{

time_t parseDateTime(const char* str)
{
    int year, month, day, hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d.%02d.%4d %02d:%02d:%02d", 
                    &day, &month, &year, &hour, &minute, &second) != 6) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = hour; dt.tm_min = minute; dt.tm_sec = second;

    //printf("%02d.%02d.%04d %02d:%02d:%02d = %ld\n", day, month, year, hour, minute, second, time);
    return mktime(&dt);
}
time_t parseDate(const char* str)
{
    int year, month, day;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d.%02d.%4d", 
                    &day, &month, &year) != 3) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = 0; dt.tm_min = 0; dt.tm_sec = 0;
    
    //printf("%02d:%02d:%04d = %ld\n", day, month, year, time);
    return mktime(&dt);
}
int parseTime(const char* str)
{
    int hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d:%02d:%02d", 
                    &hour, &minute, &second) != 3) return -1;
    
    //printf("%02d:%02d:%02d = %ld\n", hour, minute, second, time);
    return hour*3600+minute*60+second;
}

void Task::init(ConfigView* config, std::string taskName)
{
    __require__(config);

    const int MAX_PRIORITY_VALUE = 1000;

    info.name = taskName;
    info.enabled = config->getBool("enabled");
    info.priority = config->getInt("priority");
    if (info.priority <= 0 || info.priority > MAX_PRIORITY_VALUE)
        throw ConfigException("Task priority should be positive and less than %d.", 
                              MAX_PRIORITY_VALUE);
    info.retryOnFail = config->getBool("retryOnFail");
    info.replaceIfPresent = config->getBool("replaceMessage");
    info.endDate = parseDateTime(config->getString("endDate"));
    info.retryTime = parseTime(config->getString("retryTime"));
    if (info.retryOnFail && info.retryTime <= 0)
        throw ConfigException("Task retry time specified incorrectly."); 
    info.validityPeriod = parseTime(config->getString("validityPeriod"));
    info.validityDate = parseDateTime(config->getString("validityDate"));
    if (info.validityPeriod <= 0 && info.validityDate <= 0)
        throw ConfigException("Message validity period/date specified incorrectly.");
    info.activePeriodStart = parseTime(config->getString("activePeriodStart"));
    info.activePeriodEnd = parseTime(config->getString("activePeriodEnd"));
    if ((info.activePeriodStart < 0 && info.activePeriodEnd >= 0) ||
        (info.activePeriodStart >= 0 && info.activePeriodEnd < 0) ||
        (info.activePeriodStart >= 0 && info.activePeriodEnd >= 0 && 
         info.activePeriodStart >= info.activePeriodEnd))
        throw ConfigException("Task active period specified incorrectly."); 
    const char* query_sql = config->getString("query");
    if (!query_sql || query_sql[0] == '\0')
        throw ConfigException("Sql query for task empty or wasn't specified.");
    info.querySql = query_sql;
    const char* msg_template = config->getString("template");
    if (!msg_template || msg_template[0] == '\0')
        throw ConfigException("Message template for task empty or wasn't specified.");
    info.msgTemplate = msg_template;
    const char* svc_type = config->getString("svcType");
    if (info.replaceIfPresent && (!svc_type || svc_type[0] == '\0'))
        throw ConfigException("Service type task empty or wasn't specified.");
    info.svcType = svc_type;
}

bool Task::isInProcess()
{
    MutexGuard guard(inProcessLock);
    return bInProcess;
}
void Task::beginProcess()
{
    {
        MutexGuard guard(inProcessLock);
        if (bInProcess) return;
        else bInProcess = true;
    }

    while (bInProcess)
    {
        inProcessEvent.Wait(10);
        if (!bInProcess) break;

        /* TODO: 
                1) execute statement on dsOwn & get ResultSet
                2) fetch new row from it
                3) and put results into dsInt  
        */
    }
    processEndEvent.Signal();
}
void Task::endProcess()
{
    MutexGuard guard(inProcessLock);
    if (!bInProcess) return;
    
    bInProcess = false;
    inProcessEvent.Signal();
    processEndEvent.Wait();
}
void Task::doNotifyMessage(StateInfo& info)
{
}
void Task::dropAllMessages()
{
}
bool Task::getNextMessage(Connection* connection, Message& message)
{
    return false;
}

}}
