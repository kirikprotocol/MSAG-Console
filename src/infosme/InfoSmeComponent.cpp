
#include "InfoSmeComponent.h"

namespace smsc { namespace infosme 
{

using namespace smsc::core::buffers;

static const char* ARGUMENT_NAME_ID         = "id";
static const char* ARGUMENT_NAME_IDS        = "ids";
static const char* ARGUMENT_NAME_ENABLED    = "enabled";

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
    Method get_generating_tasks((unsigned)getGeneratingTasksMethod, 
                                "getGeneratingTasksMethod", empty_params, StringListType);
    Method get_processing_tasks((unsigned)getProcessingTasksMethod, 
                                "getProcessingTasksMethod", empty_params, StringListType);

    Parameters ids_params;
    ids_params[ARGUMENT_NAME_IDS] = Parameter(ARGUMENT_NAME_IDS, StringListType);
    Method add_tasks((unsigned)addTasksMethod, "addTasks", ids_params, StringType);
    Method remove_tasks((unsigned)removeTasksMethod, "removeTasks", ids_params, StringType);
    Method change_tasks((unsigned)changeTasksMethod, "changeTasks", ids_params, StringType);
    Method start_tasks((unsigned)startTasksMethod, "startTasks", ids_params, StringType);
    Method stop_tasks((unsigned)stopTasksMethod, "stopTasks", ids_params, StringType);
    Method add_schedules((unsigned)addSchedulesMethod, "addSchedules", ids_params, StringType);
    Method remove_schedules((unsigned)removeSchedulesMethod, "removeSchedules", ids_params, StringType);
    Method change_schedules((unsigned)changeSchedulesMethod, "changeSchedules", ids_params, StringType);

    Parameters task_params;
    task_params[ARGUMENT_NAME_ID] = Parameter(ARGUMENT_NAME_ID, StringType);
    Method is_task_enabled((unsigned)isTaskEnabledMethod, "isTaskEnabled",
                            task_params, BooleanType);
    task_params[ARGUMENT_NAME_ENABLED] = Parameter(ARGUMENT_NAME_ENABLED, BooleanType);
    Method set_task_enabled((unsigned)setTaskEnabledMethod, "setTaskEnabled",
                            task_params, StringType);

    methods[start_task_processor.getName()]         = start_task_processor;
    methods[stop_task_processor.getName()]          = stop_task_processor;
    methods[is_task_processor_running.getName()]    = is_task_processor_running;
    methods[start_task_scheduler.getName()]         = start_task_scheduler;
    methods[stop_task_scheduler.getName()]          = stop_task_scheduler;
    methods[is_task_scheduler_running.getName()]    = is_task_scheduler_running;
    methods[flush_statistics.getName()]             = flush_statistics;
    methods[add_tasks.getName()]                    = add_tasks;
    methods[remove_tasks.getName()]                 = remove_tasks;
    methods[change_tasks.getName()]                 = change_tasks;
    methods[start_tasks.getName()]                  = start_tasks;
    methods[stop_tasks.getName()]                   = stop_tasks;
    methods[get_generating_tasks.getName()]         = get_generating_tasks;
    methods[get_processing_tasks.getName()]         = get_processing_tasks;
    methods[is_task_enabled.getName()]              = is_task_enabled;
    methods[set_task_enabled.getName()]             = set_task_enabled;
    methods[add_schedules.getName()]                = add_schedules;
    methods[remove_schedules.getName()]             = remove_schedules;
    methods[change_schedules.getName()]             = change_schedules;
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
        case changeTasksMethod:
            changeTasks(args);
            break;
        case startTasksMethod:
            startTasks(args);
            break;
        case stopTasksMethod:
            stopTasks(args);
            break;
        case getGeneratingTasksMethod:
            return getGeneratingTasks(args);
        case getProcessingTasksMethod:
            return getProcessingTasks(args);
        case isTaskEnabledMethod:
            return Variant(isTaskEnabled(args));
        case setTaskEnabledMethod:
            setTaskEnabled(args);
            break;
        case addSchedulesMethod:
            addSchedules(args);
            break;
        case removeSchedulesMethod:
            removeSchedules(args);
            break;
        case changeSchedulesMethod:
            changeSchedules(args);
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
    if (!args.Exists(ARGUMENT_NAME_IDS)) 
        error("addTasks", ARGUMENT_NAME_IDS);
    Variant arg = args[ARGUMENT_NAME_IDS];
    if (arg.getType() != StringListType)
        error("addTasks", ARGUMENT_NAME_IDS);
    
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
    if (!args.Exists(ARGUMENT_NAME_IDS)) 
        error("removeTasks", ARGUMENT_NAME_IDS);
    Variant arg = args[ARGUMENT_NAME_IDS];
    if (arg.getType() != StringListType)
        error("removeTasks", ARGUMENT_NAME_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* taskId = *it;
        if (!taskId || taskId[0] == '\0') continue;
        if (!admin.removeTask(taskId))
            throw AdminException("Failed to remove task '%s'", taskId);
    }
}
void InfoSmeComponent::changeTasks(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_IDS)) 
        error("changeTasks", ARGUMENT_NAME_IDS);
    Variant arg = args[ARGUMENT_NAME_IDS];
    if (arg.getType() != StringListType)
        error("changeTasks", ARGUMENT_NAME_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* taskId = *it;
        if (!taskId || taskId[0] == '\0') continue;
        if (!admin.changeTask(taskId))
            throw AdminException("Failed to change task '%s'", taskId);
    }
}
void InfoSmeComponent::startTasks(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_IDS)) 
        error("startTasks", ARGUMENT_NAME_IDS);
    Variant arg = args[ARGUMENT_NAME_IDS];
    if (arg.getType() != StringListType)
        error("startTasks", ARGUMENT_NAME_IDS);
    
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
    if (!args.Exists(ARGUMENT_NAME_IDS)) 
        error("stopTasks", ARGUMENT_NAME_IDS);
    Variant arg = args[ARGUMENT_NAME_IDS];
    if (arg.getType() != StringListType)
        error("stopTasks", ARGUMENT_NAME_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* taskId = *it;
        if (!taskId || taskId[0] == '\0') continue;
        if (!admin.stopTask(taskId))
            throw AdminException("Failed to stop task '%s'", taskId);
    }
}
Variant InfoSmeComponent::getGeneratingTasks(const Arguments& args)
{
    Variant result(StringListType);
    Array<std::string> generatingTasks = admin.getGeneratingTasks();
    for (int i=0; i<generatingTasks.Count(); i++)
        result.appendValueToStringList(generatingTasks[i].c_str());
    return result;
}
Variant InfoSmeComponent::getProcessingTasks(const Arguments& args)
{
    Variant result(StringListType);
    Array<std::string> processingTasks = admin.getProcessingTasks();
    for (int i=0; i<processingTasks.Count(); i++)
        result.appendValueToStringList(processingTasks[i].c_str());
    return result;
}
bool InfoSmeComponent::isTaskEnabled(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("isTaskEnabled", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* taskId = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!taskId || taskId[0] == '\0')
        error("isTaskEnabled", ARGUMENT_NAME_ID);
    
    return admin.isTaskEnabled(taskId);
}
void InfoSmeComponent::setTaskEnabled(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("setTaskEnabled", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* taskId = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!taskId || taskId[0] == '\0')
        error("setTaskEnabled", ARGUMENT_NAME_ID);
    
    if (!args.Exists(ARGUMENT_NAME_ENABLED)) 
        error("setTaskEnabled", ARGUMENT_NAME_ENABLED);
    arg = args[ARGUMENT_NAME_ENABLED];
    if (arg.getType() != BooleanType) 
        error("setTaskEnabled", ARGUMENT_NAME_ENABLED);

    if (!admin.setTaskEnabled(taskId, arg.getBooleanValue()))
        throw AdminException("Failed to shange enabled state for task '%s'", taskId);
}
void InfoSmeComponent::addSchedules(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_IDS)) 
        error("addSchedules", ARGUMENT_NAME_IDS);
    Variant arg = args[ARGUMENT_NAME_IDS];
    if (arg.getType() != StringListType)
        error("addSchedules", ARGUMENT_NAME_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* scheduleId = *it;
        if (!scheduleId || scheduleId[0] == '\0') continue;
        if (!admin.addSchedule(scheduleId))
            throw AdminException("Failed to add schedule '%s'", scheduleId);
    }
}
void InfoSmeComponent::removeSchedules(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_IDS)) 
        error("removeSchedules", ARGUMENT_NAME_IDS);
    Variant arg = args[ARGUMENT_NAME_IDS];
    if (arg.getType() != StringListType)
        error("removeSchedules", ARGUMENT_NAME_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* scheduleId = *it;
        if (!scheduleId || scheduleId[0] == '\0') continue;
        if (!admin.removeSchedule(scheduleId))
            throw AdminException("Failed to remove schedule '%s'", scheduleId);
    }
}
void InfoSmeComponent::changeSchedules(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_IDS)) 
        error("changeSchedules", ARGUMENT_NAME_IDS);
    Variant arg = args[ARGUMENT_NAME_IDS];
    if (arg.getType() != StringListType)
        error("changeSchedules", ARGUMENT_NAME_IDS);
    
    StringList list = arg.getStringListValue();
    for (StringList::iterator it=list.begin(); it != list.end(); it++) {
        const char* scheduleId = *it;
        if (!scheduleId || scheduleId[0] == '\0') continue;
        if (!admin.changeSchedule(scheduleId))
            throw AdminException("Failed to change schedule '%s'", scheduleId);
    }
}

}}

