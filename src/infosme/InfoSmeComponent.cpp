
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
                                "startTaskProcessor", empty_params, StringType);
    Method stop_task_processor((unsigned)stopTaskProcessorMethod, 
                               "stopTaskProcessor", empty_params, StringType);
    Method is_task_processor_running((unsigned)isTaskProcessorRunningMethod, 
                                     "isTaskProcessorRunning", empty_params, BooleanType);
    Method start_task_scheduler((unsigned)startTaskSchedulerMethod, 
                                "startTaskScheduler", empty_params, StringType);
    Method stop_task_scheduler((unsigned)stopTaskSchedulerMethod, 
                               "stopTaskScheduler", empty_params, StringType);
    Method is_task_scheduler_running((unsigned)isTaskSchedulerRunningMethod, 
                                     "isTaskSchedulerRunning", empty_params, BooleanType);
    Method flush_statistics((unsigned)flushStatisticsMethod, 
                            "flushStatistics", empty_params, StringType);

    Parameters tasks_params;
    tasks_params[ARGUMENT_NAME_TASK_IDS] = Parameter(ARGUMENT_NAME_TASK_IDS, StringListType);
    Method add_tasks((unsigned)addTasksMethod, "addTasks", tasks_params, StringType);
    Method remove_tasks((unsigned)removeTasksMethod, "removeTasks", tasks_params, StringType);
    Method start_tasks((unsigned)startTasksMethod, "startTasks", tasks_params, StringType);
    Method stop_tasks((unsigned)stopTasksMethod, "stopTasks", tasks_params, StringType);

    Parameters e_task_params;
    e_task_params[ARGUMENT_NAME_TASK_ID] = Parameter(ARGUMENT_NAME_TASK_ID, StringType);
    e_task_params[ARGUMENT_NAME_ENABLED] = Parameter(ARGUMENT_NAME_ENABLED, BooleanType);
    Method set_task_enabled((unsigned)setTaskEnabledMethod, "setTaskEnabled",
                            e_task_params, StringType);

    Parameters schedule_params;
    schedule_params[ARGUMENT_NAME_SCHEDULE_ID] = Parameter(ARGUMENT_NAME_SCHEDULE_ID, StringType);
    Method add_schedule((unsigned)addScheduleMethod, "addSchedule", schedule_params, StringType);
    Method remove_schedule((unsigned)removeScheduleMethod, "removeSchedule", schedule_params, StringType);

    Parameters e_schedule_params;
    schedule_params[ARGUMENT_NAME_OLD_SCHEDULE_ID] = Parameter(ARGUMENT_NAME_OLD_SCHEDULE_ID, StringType);
    schedule_params[ARGUMENT_NAME_NEW_SCHEDULE_ID] = Parameter(ARGUMENT_NAME_NEW_SCHEDULE_ID, StringType);
    Method change_schedule((unsigned)changeScheduleMethod, "changeSchedule", e_schedule_params, StringType);

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
        case startTaskProcessorMethod:
            admin.startTaskProcessor(); 
            break;
        case stopTaskProcessorMethod:
            admin.stopTaskProcessor(); 
            break;
        case isTaskProcessorRunningMethod:
            return Variant(admin.isTaskProcessorRunning());
        case startTaskSchedulerMethod:
            admin.startTaskScheduler(); 
            break;
        case stopTaskSchedulerMethod:
            admin.stopTaskScheduler();
            break;
        case isTaskSchedulerRunningMethod:
            return Variant(admin.isTaskSchedulerRunning());
        case flushStatisticsMethod:
            admin.flushStatistics();
            break;
        case addTasksMethod:
            addTasks(args);
            break;
        case removeTasksMethod:
            removeTasks(args);
            break;
        case startTasksMethod:
            startTasks(args);
            break;
        case stopTasksMethod:
            stopTasks(args);
            break;
        case setTaskEnabledMethod:
            setTaskEnabled(args);
            break;
        case addScheduleMethod:
            addSchedule(args);
            break;
        case removeScheduleMethod:
            removeSchedule(args);
            break;
        case changeScheduleMethod:
            changeSchedule(args);
            break;
        
        default:
            logger.debug("unknown method \"%s\" [%u]", method.getName(), method.getId());
            throw AdminException("Unknown method \"%s\"", method.getName());
        }

        return Variant("");
    }
    catch (AdminException &e) {
        logger.error("AdminException: %s", e.what());
        throw e;
    }
    catch (std::exception& exc) {
        logger.error("Exception: %s", exc.what());
        throw AdminException("%s", exc.what());
    }
    catch (...) {
        logger.error("... Exception");
        throw AdminException("Unknown exception was caught during call");
    }
}

void InfoSmeComponent::error(const char* method, const char* param)
{
    throw Exception("Parameter '%s' missed or invalid for InfoSmeComponent::%s()",
                    param, method);
}

void InfoSmeComponent::addTasks(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_TASK_IDS)) 
        error("addTasks", ARGUMENT_NAME_TASK_IDS);
    Variant arg = args[ARGUMENT_NAME_TASK_IDS];
    if (arg.getType() != StringListType)
        error("addTasks", ARGUMENT_NAME_TASK_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* taskId = *it;
        if (!taskId || taskId[0] == '\0') continue;
        if (!admin.addTask(taskId))
            throw AdminException("Failed to add task '%s'", taskId);
    }
}
void InfoSmeComponent::removeTasks(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_TASK_IDS)) 
        error("removeTasks", ARGUMENT_NAME_TASK_IDS);
    Variant arg = args[ARGUMENT_NAME_TASK_IDS];
    if (arg.getType() != StringListType)
        error("removeTasks", ARGUMENT_NAME_TASK_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* taskId = *it;
        if (!taskId || taskId[0] == '\0') continue;
        if (!admin.removeTask(taskId))
            throw AdminException("Failed to remove task '%s'", taskId);
    }
}
void InfoSmeComponent::startTasks(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_TASK_IDS)) 
        error("startTasks", ARGUMENT_NAME_TASK_IDS);
    Variant arg = args[ARGUMENT_NAME_TASK_IDS];
    if (arg.getType() != StringListType)
        error("startTasks", ARGUMENT_NAME_TASK_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* taskId = *it;
        if (!taskId || taskId[0] == '\0') continue;
        if (!admin.startTask(taskId))
            throw AdminException("Failed to start task '%s'", taskId);
    }
}
void InfoSmeComponent::stopTasks(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_TASK_IDS)) 
        error("stopTasks", ARGUMENT_NAME_TASK_IDS);
    Variant arg = args[ARGUMENT_NAME_TASK_IDS];
    if (arg.getType() != StringListType)
        error("stopTasks", ARGUMENT_NAME_TASK_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* taskId = *it;
        if (!taskId || taskId[0] == '\0') continue;
        if (!admin.stopTask(taskId))
            throw AdminException("Failed to stop task '%s'", taskId);
    }
}
void InfoSmeComponent::setTaskEnabled(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_TASK_ID)) 
        error("setTaskEnabled", ARGUMENT_NAME_TASK_ID);
    Variant arg = args[ARGUMENT_NAME_TASK_ID];
    const char* taskId = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!taskId || taskId[0] == '\0')
        error("setTaskEnabled", ARGUMENT_NAME_TASK_ID);
    
    if (!args.Exists(ARGUMENT_NAME_ENABLED)) 
        error("setTaskEnabled", ARGUMENT_NAME_ENABLED);
    arg = args[ARGUMENT_NAME_ENABLED];
    if (arg.getType() != BooleanType) 
        error("setTaskEnabled", ARGUMENT_NAME_ENABLED);

    if (!admin.setTaskEnabled(taskId, arg.getBooleanValue()))
        throw AdminException("Failed to shange enabled state for task '%s'", taskId);
}
void InfoSmeComponent::addSchedule(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_SCHEDULE_ID)) 
        error("addSchedule", ARGUMENT_NAME_SCHEDULE_ID);
    Variant arg = args[ARGUMENT_NAME_SCHEDULE_ID];
    const char* scheduleId = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!scheduleId || scheduleId[0] == '\0')
        error("addSchedule", ARGUMENT_NAME_SCHEDULE_ID);

    if (!admin.addSchedule(scheduleId)) 
        throw AdminException("Failed to add schedule '%s'", scheduleId);
}
void InfoSmeComponent::removeSchedule(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_SCHEDULE_ID)) 
        error("removeSchedule", ARGUMENT_NAME_SCHEDULE_ID);
    Variant arg = args[ARGUMENT_NAME_SCHEDULE_ID];
    const char* scheduleId = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!scheduleId || scheduleId[0] == '\0')
        error("removeSchedule", ARGUMENT_NAME_SCHEDULE_ID);

    if (!admin.removeSchedule(scheduleId)) 
        throw AdminException("Failed to remove schedule '%s'", scheduleId);
}
void InfoSmeComponent::changeSchedule(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_OLD_SCHEDULE_ID)) 
        error("changeSchedule", ARGUMENT_NAME_OLD_SCHEDULE_ID);
    Variant arg = args[ARGUMENT_NAME_OLD_SCHEDULE_ID];
    const char* oldScheduleId = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!oldScheduleId || oldScheduleId[0] == '\0')
        error("changeSchedule", ARGUMENT_NAME_OLD_SCHEDULE_ID);
    
    if (!args.Exists(ARGUMENT_NAME_NEW_SCHEDULE_ID)) 
        error("changeSchedule", ARGUMENT_NAME_NEW_SCHEDULE_ID);
    arg = args[ARGUMENT_NAME_NEW_SCHEDULE_ID];
    const char* newScheduleId = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!newScheduleId || newScheduleId[0] == '\0')
        error("changeSchedule", ARGUMENT_NAME_NEW_SCHEDULE_ID);

    if (!admin.changeSchedule(oldScheduleId, newScheduleId)) 
        throw AdminException("Failed to change schedule '%s' to '%s'", oldScheduleId, newScheduleId);
}

}}

