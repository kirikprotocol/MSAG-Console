
#include "InfoSmeComponent.h"

namespace smsc { namespace infosme 
{

using namespace smsc::core::buffers;

static const char* ARGUMENT_NAME_ID         = "id";
static const char* ARGUMENT_NAME_IDS        = "ids";
static const char* ARGUMENT_NAME_ENABLED    = "enabled";

InfoSmeComponent::InfoSmeComponent(InfoSmeAdmin& admin)
    : logger(Logger::getInstance("smsc.infosme.InfoSmeComponent")), admin(admin)
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

    Parameters id_params;
    id_params[ARGUMENT_NAME_ID] = Parameter(ARGUMENT_NAME_ID, StringType);
    Method add_task((unsigned)addTaskMethod, "addTask", id_params, StringType);
    Method remove_task((unsigned)removeTaskMethod, "removeTask", id_params, StringType);
    Method change_task((unsigned)changeTaskMethod, "changeTask", id_params, StringType);
    Method add_schedule((unsigned)addScheduleMethod, "addSchedule", id_params, StringType);
    Method remove_schedule((unsigned)removeScheduleMethod, "removeSchedule", id_params, StringType);
    Method change_schedule((unsigned)changeScheduleMethod, "changeSchedule", id_params, StringType);

    Parameters ids_params;
    ids_params[ARGUMENT_NAME_IDS] = Parameter(ARGUMENT_NAME_IDS, StringListType);
    Method start_tasks((unsigned)startTasksMethod, "startTasks", ids_params, StringType);
    Method stop_tasks((unsigned)stopTasksMethod, "stopTasks", ids_params, StringType);

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
    methods[add_task.getName()]                     = add_task;
    methods[remove_task.getName()]                  = remove_task;
    methods[change_task.getName()]                  = change_task;
    methods[start_tasks.getName()]                  = start_tasks;
    methods[stop_tasks.getName()]                   = stop_tasks;
    methods[get_generating_tasks.getName()]         = get_generating_tasks;
    methods[get_processing_tasks.getName()]         = get_processing_tasks;
    methods[is_task_enabled.getName()]              = is_task_enabled;
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
        case addTaskMethod:
            addTask(args);
            break;
        case removeTaskMethod:
            removeTask(args);
            break;
        case changeTaskMethod:
            changeTask(args);
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

void InfoSmeComponent::addTask(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("addTask", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("addTask", ARGUMENT_NAME_ID);
    
    try { admin.addTask(id);
    } catch (Exception& exc) {
        throw AdminException("Failed to add task '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to add task '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to add task '%s'. Cause is unknown", id);
    }
}
void InfoSmeComponent::removeTask(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("removeTask", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("removeTask", ARGUMENT_NAME_ID);
    
    try { admin.removeTask(id);
    } catch (Exception& exc) {
        throw AdminException("Failed to remove task '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to remove task '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to remove task '%s'. Cause is unknown", id);
    }
}
void InfoSmeComponent::changeTask(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("changeTask", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("changeTask", ARGUMENT_NAME_ID);
    
    try { admin.changeTask(id);
    } catch (Exception& exc) {
        throw AdminException("Failed to change task '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to change task '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to change task '%s'. Cause is unknown", id);
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
void InfoSmeComponent::addSchedule(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("addSchedule", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("addSchedule", ARGUMENT_NAME_ID);
    
    try { admin.addSchedule(id);
    } catch (Exception& exc) {
        throw AdminException("Failed to add schedule '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to add schedule '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to add schedule '%s'. Cause is unknown", id);
    }
}
void InfoSmeComponent::removeSchedule(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("removeSchedule", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("removeSchedule", ARGUMENT_NAME_ID);
    
    try { admin.removeSchedule(id);
    } catch (Exception& exc) {
        throw AdminException("Failed to remove schedule '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to remove schedule '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to remove schedule '%s'. Cause is unknown", id);
    }
}
void InfoSmeComponent::changeSchedule(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("changeSchedule", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("changeSchedule", ARGUMENT_NAME_ID);
    
    try { admin.changeSchedule(id);
    } catch (Exception& exc) {
        throw AdminException("Failed to change schedule '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to change schedule '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to change schedule '%s'. Cause is unknown", id);
    }
}

}}

