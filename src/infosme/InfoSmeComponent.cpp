
#include "InfoSmeComponent.h"

namespace smsc { namespace infosme 
{

using namespace smsc::core::buffers;

static const char* ARGUMENT_NAME_TASK_ID            = "taskId";
static const char* ARGUMENT_NAME_TASK_IDS           = "taskIds";
static const char* ARGUMENT_NAME_ENABLED            = "enabled";
static const char* ARGUMENT_NAME_SCHEDULE_ID        = "scheduleId";
static const char* ARGUMENT_NAME_OLD_SCHEDULE_ID    = "oldScheduleId";
static const char* ARGUMENT_NAME_NEW_SCHEDULE_ID    = "newScheduleId";

InfoSmeComponent::InfoSmeComponent(InfoSmeAdmin& admin)
    : logger(Logger::getCategory("smsc.infosme.InfoSmeComponent")), admin(admin)
{
    Parameters empty_params;
    Method start_task_processor((unsigned)startTaskProcessorMethod, 
                                "start_task_processor", empty_params, StringType);
    Method stop_task_processor((unsigned)stopTaskProcessorMethod, 
                               "stop_task_processor", empty_params, StringType);
    Method is_task_processor_running((unsigned)isTaskProcessorRunningMethod, 
                                     "is_task_processor_running", empty_params, BooleanType);
    Method start_task_scheduler((unsigned)startTaskSchedulerMethod, 
                                "start_task_scheduler", empty_params, StringType);
    Method stop_task_scheduler((unsigned)stopTaskSchedulerMethod, 
                               "stop_task_scheduler", empty_params, StringType);
    Method is_task_scheduler_running((unsigned)isTaskSchedulerRunningMethod, 
                                     "is_task_scheduler_running", empty_params, BooleanType);
    Method flush_statistics((unsigned)flushStatisticsMethod, 
                            "flush_statistics", empty_params, StringType);

    Parameters tasks_params;
    tasks_params[ARGUMENT_NAME_TASK_IDS] = Parameter(ARGUMENT_NAME_TASK_IDS, StringListType);
    Method add_tasks((unsigned)addTasksMethod, "add_tasks", tasks_params, StringType);
    Method remove_tasks((unsigned)removeTasksMethod, "remove_tasks", tasks_params, StringType);
    Method start_tasks((unsigned)startTasksMethod, "start_tasks", tasks_params, StringType);
    Method stop_tasks((unsigned)stopTasksMethod, "stop_tasks", tasks_params, StringType);

    Parameters e_task_params;
    e_task_params[ARGUMENT_NAME_TASK_ID] = Parameter(ARGUMENT_NAME_TASK_ID, StringType);
    e_task_params[ARGUMENT_NAME_ENABLED] = Parameter(ARGUMENT_NAME_ENABLED, BooleanType);
    Method set_task_enabled((unsigned)setTaskEnabledMethod, "set_task_enabled",
                            e_task_params, StringType);

    Parameters schedule_params;
    schedule_params[ARGUMENT_NAME_SCHEDULE_ID] = Parameter(ARGUMENT_NAME_SCHEDULE_ID, StringType);
    Method add_schedule((unsigned)addScheduleMethod, "add_schedule", schedule_params, StringType);
    Method remove_schedule((unsigned)removeScheduleMethod, "remove_schedule", schedule_params, StringType);

    Parameters e_schedule_params;
    schedule_params[ARGUMENT_NAME_OLD_SCHEDULE_ID] = Parameter(ARGUMENT_NAME_OLD_SCHEDULE_ID, StringType);
    schedule_params[ARGUMENT_NAME_NEW_SCHEDULE_ID] = Parameter(ARGUMENT_NAME_NEW_SCHEDULE_ID, StringType);
    Method change_schedule((unsigned)changeScheduleMethod, "change_schedule", e_schedule_params, StringType);

    methods[start_task_processor.getName()]         = start_task_processor;
    methods[stop_task_processor.getName()]          = stop_task_processor;
    methods[is_task_processor_running.getName()]    = is_task_processor_running;
    methods[start_task_scheduler.getName()]         = start_task_scheduler;
    methods[stop_task_scheduler.getName()]          = stop_task_scheduler;
    methods[is_task_scheduler_running.getName()]    = is_task_scheduler_running;
    methods[flush_statistics.getName()]             = flush_statistics;
    methods[add_tasks.getName()]                    = add_tasks;
    methods[remove_tasks.getName()]                 = remove_tasks;
    methods[start_tasks.getName()]                  = start_tasks;
    methods[stop_tasks.getName()]                   = stop_tasks;
    methods[set_task_enabled.getName()]             = set_task_enabled;
    methods[add_schedule.getName()]                 = add_schedule;
    methods[remove_schedule.getName()]              = remove_schedule;
    methods[change_schedule.getName()]              = change_schedule;
}

InfoSmeComponent::~InfoSmeComponent()
{
}

Variant InfoSmeComponent::call(const Method& method, const Arguments& args)
    throw (AdminException)
{
    try 
    {
        logger.debug("call \"%s\"", method.getName());
        
        switch (method.getId())
        {
        case addTasksMethod:
            logger.debug("addTaskMethod processing...");
            logger.debug("addTaskMethod processed.");
            break;
        case removeTasksMethod:
            logger.debug("removeTaskMethod processing...");
            logger.debug("removeTaskMethod processed.");
            break;

        default:
            logger.debug("unknown method \"%s\" [%u]", method.getName(), method.getId());
            throw AdminException("Unknown method \"%s\"", method.getName());
        }

        return Variant("");
    }
    catch (AdminException &e) {
        logger.debug("AdminException: %s", e.what());
        throw e;
    }
    catch (...) {
        logger.debug("... Exception");
        throw AdminException("Unknown exception catched during call");
    }
}

void InfoSmeComponent::error(const char* method, const char* param)
{
    Exception exc("Parameter '%s' missed or invalid "
                  "for InfoSmeComponent::%s()", param, method);
    logger.error(exc.what());
    throw exc;
}

}}

