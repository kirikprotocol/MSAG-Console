#include "InfoSmeComponent.h"
#include "InfoSme_Tasks_Stat_SearchCriterion.hpp"
#include "DateTime.h"
#include "Task.h"

namespace smsc { 
namespace infosme {

using namespace smsc::core::buffers;

static const char* ARGUMENT_NAME_ID         = "id";
static const char* ARGUMENT_NAME_IDS        = "ids";
static const char* ARGUMENT_NAME_ENABLED    = "enabled";
static const char* ARGUMENT_STATE           = "state";
static const char* ARGUMENT_ADDRESS         = "address";
static const char* ARGUMENT_DATE            = "date";
static const char* ARGUMENT_MESSAGES        = "messages";

static const char* ARGUMENT_RECORD_ID       = "record_id";
static const char* ARGUMENT_OLD_STATE       = "old_state";
static const char* ARGUMENT_FROM_DATE       = "from_date";
static const char* ARGUMENT_TO_DATE         = "to_date";
static const char* ARGUMENT_PERIOD          = "period";
static const char* ARGUMENT_GENERATED       = "generated";
static const char* ARGUMENT_DELIVERED       = "delivered";
static const char* ARGUMENT_RETRIED         = "retried";
static const char* ARGUMENT_FAILED          = "failed";
static const char* ARGUMENT_ORDER_BY        = "order_by";
static const char* ARGUMENT_ORDER_DIRECTION = "order_direction";
static const char* ARGUMENT_MSG_LIMIT       = "msg_limit";

static const char* ARGUMENT_START_PERIOD    = "start_period";
static const char* ARGUMENT_END_PERIOD      = "end_period";
static const char* ARGUMENT_NEW_TEXT_MESSAGE = "new_text_message";

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
  Method apply_retry_policies((unsigned)applyRetryPoliciesMethod,"applyRetryPolicies",empty_params,StringType);

  Parameters id_params;
  id_params[ARGUMENT_NAME_ID] = Parameter(ARGUMENT_NAME_ID, StringType);
  Method add_task((unsigned)addTaskMethod, "addTask", id_params, StringType);
  Method remove_task((unsigned)removeTaskMethod, "removeTask", id_params, StringType);
  Method change_task((unsigned)changeTaskMethod, "changeTask", id_params, StringType);
  Method add_schedule((unsigned)addScheduleMethod, "addSchedule", id_params, StringType);
  Method remove_schedule((unsigned)removeScheduleMethod, "removeSchedule", id_params, StringType);
  Method change_schedule((unsigned)changeScheduleMethod, "changeSchedule", id_params, StringType);
  Method end_delivery_messages_generation((unsigned)endDeliveryMessagesGenerationMethod, "endDeliveryMessagesGeneration", id_params, StringType);
  
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
  
  // interface to add delivery message
  Parameters delivery_message_params;
  delivery_message_params[ARGUMENT_NAME_ID] = Parameter(ARGUMENT_NAME_ID, StringType);
  /*  delivery_message_params[ARGUMENT_STATE] = Parameter(ARGUMENT_STATE, LongType);
   delivery_message_params[ARGUMENT_ADDRESS] = Parameter(ARGUMENT_ADDRESS, StringType);
   // date format is DDMMYYYYHH24MISS
   delivery_message_params[ARGUMENT_DATE] = Parameter(ARGUMENT_DATE, StringType);*/
  // list of messages
  delivery_message_params[ARGUMENT_MESSAGES] = Parameter(ARGUMENT_MESSAGES, StringListType);
  
  Method add_delivery_message((unsigned)addDeliveryMessagesMethod, "addDeliveryMessages",
                              delivery_message_params, StringType);
  
  // interface to change state and send date of specified delivery messages
  Parameters change_delivery_message_params;
  change_delivery_message_params[ARGUMENT_NAME_ID]   = Parameter(ARGUMENT_NAME_ID, StringType);
  // next two argmunets is new state and send date of modified record
  change_delivery_message_params[ARGUMENT_STATE]     = Parameter(ARGUMENT_STATE, LongType);
  // date format is DDMMYYYYHH24MISS
  change_delivery_message_params[ARGUMENT_DATE]      = Parameter(ARGUMENT_DATE, StringType);
  
  // next arguments needs for searching of updatable records
  change_delivery_message_params[ARGUMENT_RECORD_ID] = Parameter(ARGUMENT_RECORD_ID, StringType);
  
  change_delivery_message_params[ARGUMENT_ADDRESS]   = Parameter(ARGUMENT_ADDRESS, StringType);
  change_delivery_message_params[ARGUMENT_OLD_STATE] = Parameter(ARGUMENT_OLD_STATE, StringType);
  change_delivery_message_params[ARGUMENT_FROM_DATE] = Parameter(ARGUMENT_FROM_DATE, StringType);
  change_delivery_message_params[ARGUMENT_TO_DATE]   = Parameter(ARGUMENT_TO_DATE, StringType);
  Method change_delivery_message_info((unsigned)changeDeliveryMessageInfoMethod, "changeDeliveryMessageInfo",
                                      change_delivery_message_params, StringType);
  
  // interface to delete delivery message(s)
  Parameters delete_delivery_messages_params;
  delete_delivery_messages_params[ARGUMENT_NAME_ID]   = Parameter(ARGUMENT_NAME_ID, StringType);
  delete_delivery_messages_params[ARGUMENT_RECORD_ID] = Parameter(ARGUMENT_RECORD_ID, StringType);
  delete_delivery_messages_params[ARGUMENT_STATE]     = Parameter(ARGUMENT_STATE, StringType);
  delete_delivery_messages_params[ARGUMENT_FROM_DATE] = Parameter(ARGUMENT_FROM_DATE, StringType);
  delete_delivery_messages_params[ARGUMENT_TO_DATE]   = Parameter(ARGUMENT_TO_DATE, StringType);
  delete_delivery_messages_params[ARGUMENT_ADDRESS]   = Parameter(ARGUMENT_ADDRESS, StringType);
  Method delete_delivery_messages((unsigned)deleteDeliveryMessagesMethod, "deleteDeliveryMessages",
                                  delete_delivery_messages_params, StringType);
  
  // intrerface to add statistic record
  /*Parameters add_statistic_record_params;
   add_statistic_record_params[ARGUMENT_NAME_ID]      = Parameter(ARGUMENT_NAME_ID, StringType);
   add_statistic_record_params[ARGUMENT_PERIOD]       = Parameter(ARGUMENT_PERIOD, LongType);
   add_statistic_record_params[ARGUMENT_GENERATED]    = Parameter(ARGUMENT_GENERATED, LongType);
   add_statistic_record_params[ARGUMENT_DELIVERED]    = Parameter(ARGUMENT_DELIVERED, LongType);
   add_statistic_record_params[ARGUMENT_RETRIED]      = Parameter(ARGUMENT_RETRIED, LongType);
   add_statistic_record_params[ARGUMENT_FAILED]       = Parameter(ARGUMENT_FAILED, LongType);
   Method add_statistic_record ((unsigned)addStatisticRecordMethod, "addStatisticRecord",
   add_statistic_record_params, StringType);*/
  
  // interface for select delivery messages
  Parameters message_criterion_params;
  message_criterion_params[ARGUMENT_NAME_ID]         = Parameter(ARGUMENT_NAME_ID, StringType);
  message_criterion_params[ARGUMENT_STATE]           = Parameter(ARGUMENT_STATE, StringType);
  message_criterion_params[ARGUMENT_FROM_DATE]       = Parameter(ARGUMENT_FROM_DATE, StringType);
  message_criterion_params[ARGUMENT_TO_DATE]         = Parameter(ARGUMENT_TO_DATE, StringType);
  message_criterion_params[ARGUMENT_ORDER_BY]        = Parameter(ARGUMENT_ORDER_BY, StringType);
  message_criterion_params[ARGUMENT_ORDER_DIRECTION] = Parameter(ARGUMENT_ORDER_DIRECTION, StringType);
  message_criterion_params[ARGUMENT_ADDRESS]         = Parameter(ARGUMENT_ADDRESS, StringType);
  message_criterion_params[ARGUMENT_MSG_LIMIT]       = Parameter(ARGUMENT_MSG_LIMIT, StringType);
  Method select_task_messages ((unsigned)selectTaskMessagesMethod, "selectTaskMessages",
                               message_criterion_params, StringListType);
  
  //    
  Parameters select_statistic_params;
  // all parameters is optional
  select_statistic_params[ARGUMENT_NAME_ID]         = Parameter(ARGUMENT_NAME_ID, StringType);
  select_statistic_params[ARGUMENT_START_PERIOD]    = Parameter(ARGUMENT_START_PERIOD, StringType);
  select_statistic_params[ARGUMENT_END_PERIOD]      = Parameter(ARGUMENT_END_PERIOD, StringType);
  Method select_tasks_statistic ((unsigned)selectTasksStatisticMethod, "selectTasksStatistic",
                                 select_statistic_params, StringListType);
  
  // interface to change text of delivery message(s)
  Parameters change_delivery_text_message_params;
  change_delivery_text_message_params[ARGUMENT_NAME_ID]   = Parameter(ARGUMENT_NAME_ID, StringType);
  change_delivery_text_message_params[ARGUMENT_NEW_TEXT_MESSAGE] = Parameter(ARGUMENT_NEW_TEXT_MESSAGE, StringType);
  change_delivery_text_message_params[ARGUMENT_RECORD_ID] = Parameter(ARGUMENT_RECORD_ID, StringType);
  change_delivery_text_message_params[ARGUMENT_STATE]     = Parameter(ARGUMENT_STATE, StringType);
  change_delivery_text_message_params[ARGUMENT_FROM_DATE] = Parameter(ARGUMENT_FROM_DATE, StringType);
  change_delivery_text_message_params[ARGUMENT_TO_DATE]   = Parameter(ARGUMENT_TO_DATE, StringType);
  change_delivery_text_message_params[ARGUMENT_ADDRESS]   = Parameter(ARGUMENT_ADDRESS, StringType);
  Method change_delivery_text_message((unsigned)changeDeliveryTextMessageMethod, "changeDeliveryTextMessage",
                                      change_delivery_text_message_params, StringType);
  
  Parameters send_sms_params;
  send_sms_params["source"] = Parameter("source",StringType);
  send_sms_params["destination"] = Parameter("destination",StringType);
  send_sms_params["message"] = Parameter("message",StringType);
  send_sms_params["flash"] = Parameter("flash",BooleanType);
  Method send_sms((unsigned)sendSmsMethod,"sendSms",send_sms_params,LongType);

    Method reload_smsc_and_regions((unsigned)reloadSmscAndRegionsMethod,
                                   "reloadSmscAndRegions", empty_params, StringType );

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
  methods[add_delivery_message.getName()]         = add_delivery_message;
  methods[change_delivery_message_info.getName()] = change_delivery_message_info;
  methods[delete_delivery_messages.getName()]     = delete_delivery_messages;
  //methods[add_statistic_record.getName()]         = add_statistic_record;
  methods[select_task_messages.getName()]         = select_task_messages;
  methods[select_tasks_statistic.getName()]       = select_tasks_statistic;
  methods[end_delivery_messages_generation.getName()] = end_delivery_messages_generation;
  methods[change_delivery_text_message.getName()]     = change_delivery_text_message;
  methods[apply_retry_policies.getName()] = apply_retry_policies;
  methods[send_sms.getName()] = send_sms;
    methods[reload_smsc_and_regions.getName()] = reload_smsc_and_regions;
}

InfoSmeComponent::~InfoSmeComponent()
{
}

Variant InfoSmeComponent::call(const Method& method, const Arguments& args)
    throw (AdminException)
{
    try 
    {
        //smsc_log_debug(logger, "call \"%s\"", method.getName());
        
        if ( ! admin.isInited() ) {
            throw AdminException("TaskProcessor is not inited yet, please wait");
        }

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
        case addDeliveryMessagesMethod:
          addDeliveryMessages(args);
          break;
        case changeDeliveryMessageInfoMethod:
          changeDeliveryMessageInfo(args);
          break;
        case deleteDeliveryMessagesMethod:
          deleteDeliveryMessages(args);
          break;
        //case addStatisticRecordMethod:
          //addStatisticRecord(args);
          //break;
        case selectTaskMessagesMethod:
          return selectTaskMessages(args);
        case selectTasksStatisticMethod:
          return selectTasksStatistic(args);
        case endDeliveryMessagesGenerationMethod:
          endDeliveryMessagesGeneration(args);
          break;
        case changeDeliveryTextMessageMethod:
          changeDeliveryTextMessage(args);
          break;
        case applyRetryPoliciesMethod:
          applyRetryPolicies(args);
          break;
        case sendSmsMethod:
          return sendSms(args);
        case reloadSmscAndRegionsMethod:
            reloadSmscAndRegions(args);
            break;
        default:
            smsc_log_debug(logger, "unknown method \"%s\" [%u]", method.getName(), method.getId());
            throw AdminException("Unknown method \"%s\"", method.getName());
        }

        return Variant("");
    }
    catch (AdminException &e) {
        smsc_log_error(logger, "AdminException: %s", e.what());
        throw;
    }
    catch (std::exception& exc) {
        smsc_log_error(logger, "Exception: %s", exc.what());
        throw AdminException("%s", exc.what());
    }
    catch (...) {
        smsc_log_error(logger, "... Exception");
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
    
    try {
        uint32_t tid = atoi(id);
        if ( admin.getTask(tid).get() ) throw Exception("task %u exists already");
        admin.initTask(tid,0);
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

    try {
        // remove task from processor
        TaskGuard task(admin.getTask(atoi(id),true));
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
    
    try {
        uint32_t tid = atoi(id);
        if ( ! admin.getTask(tid).get() ) throw Exception("task %u is not found",tid);
        admin.initTask(tid,0);
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
        if (!admin.startTask(atoi(taskId)))
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
        if (!admin.stopTask(atoi(taskId)))
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
    
    return admin.isTaskEnabled(atoi(taskId));
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

    if (!admin.setTaskEnabled(atoi(taskId), arg.getBooleanValue()))
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

void InfoSmeComponent::deleteEscapeSymbols(std::string* message)
{
  std::string::size_type pos=0;
  while ((pos = message->find('\\'), pos) != std::string::npos)
    message->erase(pos++,1);
}

void InfoSmeComponent::splitMessageToFields(const std::string& messageDescription, long* messageState,
                                            std::string* address, std::string* messageDate,
                                            std::string* messageText, std::string* userData )
{
  std::string::size_type prevPos = 0;
  std::string::size_type pos = messageDescription.find('|');
  if ( pos == std::string::npos )
    error("splitMessageToFields", ARGUMENT_STATE);

  *messageState = atoi(messageDescription.substr(prevPos, pos).c_str()) ;

  prevPos = pos + 1;
  pos = messageDescription.find('|', prevPos);
  if ( pos == std::string::npos )
    error("splitMessageToFields", ARGUMENT_ADDRESS);
  *address = messageDescription.substr(prevPos, pos-prevPos);

  prevPos = pos + 1;
  pos = messageDescription.find('|', prevPos);
  if ( pos == std::string::npos )
    error("splitMessageToFields", ARGUMENT_DATE);

  *messageDate = messageDescription.substr(prevPos, pos-prevPos);

  prevPos = pos + 1;
  pos = messageDescription.find('|',prevPos);
  if ( pos != std::string::npos ) {
      // we have user data
      *userData = messageDescription.substr(prevPos,pos-prevPos);
      prevPos = pos + 1;
  }
  *messageText = std::string(messageDescription, prevPos);
}

/*
INSERT INTO %
(ID, STATE, ABONENT, SEND_DATE, MESSAGE) VALUES (INFOSME_MESSAGES_SEQ.NEXTVAL, ?, ?, ?, ?);
*/
void InfoSmeComponent::addDeliveryMessages(const Arguments& args)
{
  try {
    if (!args.Exists(ARGUMENT_NAME_ID)) 
      error("addDeliveryMessages", ARGUMENT_NAME_ID);

    Variant arg = args[ARGUMENT_NAME_ID];

    const std::string taskId = (arg.getType() == StringType) ? arg.getStringValue():"";
    if (taskId == "")
      error("addDeliveryMessages", ARGUMENT_NAME_ID);

    if ( !args.Exists(ARGUMENT_MESSAGES) )
      error("addDeliveryMessages", ARGUMENT_MESSAGES);

    arg = args[ARGUMENT_MESSAGES];
    if (arg.getType() != StringListType)
      error("addDeliveryMessages", ARGUMENT_MESSAGES);

    const StringList list(arg.getStringListValue());
    smsc_log_info(logger, "InfoSmeComponent::addDeliveryMessages::: taskId=[%s], num. of new messages=[%d]",taskId.c_str(), list.size());
    for (StringList::const_iterator it=list.begin(); it != list.end(); it++) {
      long messageState;
      std::string address, messageDate, messageText, userData;

      splitMessageToFields(*it, &messageState, &address, &messageDate, &messageText, &userData);

      if (address == "")
        error("addDeliveryMessages", ARGUMENT_ADDRESS);

      time_t unixTime;
      if (messageDate == "" ||
          !convertFullDateFormatToUnixTime(messageDate, &unixTime))
        error("addDeliveryMessages", ARGUMENT_DATE);

      deleteEscapeSymbols(&messageText);
      if (!userData.empty()) {
          deleteEscapeSymbols(&userData);
      }
      admin.addDeliveryMessages(atoi(taskId.c_str()), (uint8_t)messageState, address, unixTime, messageText, userData);
    }
    smsc_log_info(logger, "InfoSmeComponent::addDeliveryMessages::: messages have been loaded");
  } catch (std::exception& exc) {
    throw AdminException("Failed to add delivery message. Cause: %s", exc.what());
  } catch (...) {
    throw AdminException("Failed to add delivery message.");
  }

}

#include "InfoSme_T_SearchCriterion.hpp"

//template <class T>
bool
InfoSmeComponent::getParameterIfExistsAndNotNull(const Arguments& args, const char* argumentId,
                                                 std::string& argumentValue)
{
  if ( args.Exists(argumentId) ) {
    Variant arg = args[argumentId];
    const std::string tempValue = (arg.getType() == StringType) ? arg.getStringValue() : "";
    if ( tempValue != "" ) {
      argumentValue = tempValue;
      return true;
    } else return false;
  } else return false;
}

/*
UPDATE % SET state=?, send_date=? 
[WHERE [abonent = ? AND] [state = ? AND] [send_date >= ? AND ] [ send_date <= ?]]

UPDATE % SET state=?, send_date=? WHERE id=?
 */
void InfoSmeComponent::changeDeliveryMessageInfo(const Arguments& args)
{
  try {
    std::string taskId;
    if ( !getParameterIfExistsAndNotNull(args, ARGUMENT_NAME_ID, taskId) )
      error("changeDeliveryMessageInfo", ARGUMENT_NAME_ID);

    smsc_log_debug(logger, "InfoSmeComponent::changeDeliveryMessageInfo::: taskId=[%s]",taskId.c_str());

    if ( !args.Exists(ARGUMENT_STATE) )
      error("changeDeliveryMessageInfo", ARGUMENT_STATE);

    Variant arg = args[ARGUMENT_STATE];
    if ( arg.getType() != LongType )
      error("changeDeliveryMessageInfo", ARGUMENT_STATE);

    uint8_t messageState = (uint8_t)arg.getLongValue();

    std::string messageDate;
    time_t unixTime;
    if ( !getParameterIfExistsAndNotNull(args, ARGUMENT_DATE, messageDate) ||
         !convertFullDateFormatToUnixTime(messageDate, &unixTime))
      error("changeDeliveryMessageInfo", ARGUMENT_DATE);

    std::string recordId;
    if ( getParameterIfExistsAndNotNull(args, ARGUMENT_RECORD_ID, recordId) )
      admin.changeDeliveryMessageInfoByRecordId(atoi(taskId.c_str()), messageState, unixTime, recordId);
    else {
      InfoSme_T_SearchCriterion searchCrit;

      std::string address;
      if ( getParameterIfExistsAndNotNull(args, ARGUMENT_ADDRESS, address) )
        searchCrit.setAbonentAddress(address);

      std::string oldStateAsString;
      if ( getParameterIfExistsAndNotNull(args, ARGUMENT_OLD_STATE, oldStateAsString) ) {
        searchCrit.setState((uint8_t)atoi(oldStateAsString.c_str()));
      }

      std::string fromDate;
      if ( getParameterIfExistsAndNotNull(args, ARGUMENT_FROM_DATE, fromDate) ) {
        time_t fromDateAsUnixTime;
        if (!convertFullDateFormatToUnixTime(fromDate, &fromDateAsUnixTime))
          error("changeDeliveryMessageInfo", ARGUMENT_FROM_DATE);

        searchCrit.setFromDate(fromDateAsUnixTime);
      }

      std::string toDate;
      if ( getParameterIfExistsAndNotNull(args, ARGUMENT_TO_DATE, toDate) ) {
        time_t toDateAsUnixTime;
        if (!convertFullDateFormatToUnixTime(toDate, &toDateAsUnixTime))
          error("changeDeliveryMessageInfo", ARGUMENT_TO_DATE);

        searchCrit.setToDate(toDateAsUnixTime);
      }
      admin.changeDeliveryMessageInfoByCompositCriterion(atoi(taskId.c_str()), messageState, unixTime, searchCrit);
    }
  } catch (std::exception& exc) {
    throw AdminException("Failed to change delivery message info. Cause: %s", exc.what());
  } catch (...) {
    throw AdminException("Failed to change delivery message info.");
  }
}

/*
DELETE FROM % 
WHERE [state = ? AND] [send_date >= ? AND ] [ send_date <= ?]

DELETE FROM % WHERE id=?
*/
void InfoSmeComponent::deleteDeliveryMessages(const Arguments& args)
{
  std::string taskId;
  if ( !getParameterIfExistsAndNotNull(args, ARGUMENT_NAME_ID, taskId) )
    error("deleteDeliveryMessages", ARGUMENT_NAME_ID);

  smsc_log_debug(logger, "InfoSmeComponent::deleteDeliveryMessages::: taskId=[%s]",taskId.c_str());

  std::string recordId;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_RECORD_ID, recordId) )
    admin.deleteDeliveryMessageByRecordId(atoi(taskId.c_str()), recordId);
  else {
    InfoSme_T_SearchCriterion searchCrit;

    std::string stateAsString;
    if ( getParameterIfExistsAndNotNull(args, ARGUMENT_STATE, stateAsString) ) {
      searchCrit.setState((uint8_t)atoi(stateAsString.c_str()));
    }
    std::string fromDate;
    if ( getParameterIfExistsAndNotNull(args, ARGUMENT_FROM_DATE, fromDate) ) {
      time_t fromDateAsUnixTime;
      if (!convertFullDateFormatToUnixTime(fromDate, &fromDateAsUnixTime))
        error("deleteDeliveryMessages", ARGUMENT_FROM_DATE);

      searchCrit.setFromDate(fromDateAsUnixTime);
    }
    std::string toDate;
    if ( getParameterIfExistsAndNotNull(args, ARGUMENT_TO_DATE, toDate ) ) {
      time_t toDateAsUnixTime;
      if (!convertFullDateFormatToUnixTime(toDate, &toDateAsUnixTime))
        error("deleteDeliveryMessages", ARGUMENT_TO_DATE);

      searchCrit.setToDate(toDateAsUnixTime);
    }
    std::string address;
    if ( getParameterIfExistsAndNotNull(args, ARGUMENT_ADDRESS, address) )
      searchCrit.setAbonentAddress(address);

    admin.deleteDeliveryMessagesByCompositCriterion(atoi(taskId.c_str()), searchCrit);
  }
}

/*void InfoSmeComponent::addStatisticRecord(const Arguments& args)
{
  uint32_t period, generated, delivered, retried, failed;

  std::string taskId;
  if ( !getParameterIfExistsAndNotNull(args, ARGUMENT_NAME_ID, taskId) )
    error("addStatisticRecord", ARGUMENT_NAME_ID);

  smsc_log_debug(logger, "InfoSmeComponent::addStatisticRecord::: taskId=[%s]",taskId.c_str());

  Variant arg;
  if ( args.Exists(ARGUMENT_PERIOD) ) {
    arg = args[ARGUMENT_PERIOD];
    if ( arg.getType() != LongType )
      error("addStatisticRecord", ARGUMENT_PERIOD);

    period = arg.getLongValue();
  }

  if ( args.Exists(ARGUMENT_GENERATED) ) {
    arg = args[ARGUMENT_GENERATED];
    if ( arg.getType() != LongType )
      error("addStatisticRecord", ARGUMENT_GENERATED);

    generated = arg.getLongValue();
  }

  if ( args.Exists(ARGUMENT_DELIVERED) ) {
    arg = args[ARGUMENT_DELIVERED];
    if ( arg.getType() != LongType )
      error("addStatisticRecord", ARGUMENT_DELIVERED);

    delivered = arg.getLongValue();
  }

  if ( args.Exists(ARGUMENT_RETRIED) ) {
    arg = args[ARGUMENT_RETRIED];
    if ( arg.getType() != LongType )
      error("addStatisticRecord", ARGUMENT_RETRIED);

    retried = arg.getLongValue();
  }

  if ( args.Exists(ARGUMENT_FAILED) ) {
    arg = args[ARGUMENT_FAILED];
    if ( arg.getType() != LongType )
      error("addStatisticRecord", ARGUMENT_FAILED);

    failed = arg.getLongValue();
  }

  admin.insertRecordIntoTasksStat(atoi(taskId.c_str()), period, generated, delivered, retried, failed);
}*/

Variant InfoSmeComponent::selectTaskMessages(const Arguments& args)
{
  std::string taskId;
  if ( !getParameterIfExistsAndNotNull(args, ARGUMENT_NAME_ID, taskId ) )
    error("selectTaskMessages", ARGUMENT_NAME_ID);
  smsc_log_debug(logger, "InfoSmeComponent::selectTaskMessages::: taskId=[%s]",taskId.c_str());
  
  InfoSme_T_SearchCriterion searchCrit;

  std::string stateAsString;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_STATE, stateAsString) ) {
    searchCrit.setState((uint8_t)atoi(stateAsString.c_str()));
  }

  std::string fromDate;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_FROM_DATE, fromDate) ) {
    time_t fromDateAsUnixTime;
    if (!convertFullDateFormatToUnixTime(fromDate, &fromDateAsUnixTime))
      error("selectTaskMessages", ARGUMENT_FROM_DATE);

    searchCrit.setFromDate(fromDateAsUnixTime);
  }

  std::string toDate;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_TO_DATE, toDate) ) {
    time_t toDateAsUnixTime;
    if (!convertFullDateFormatToUnixTime(toDate, &toDateAsUnixTime))
      error("selectTaskMessages", ARGUMENT_TO_DATE);

    searchCrit.setToDate(toDateAsUnixTime);
  }
  
  std::string orderBy;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_ORDER_BY, orderBy) )
    searchCrit.setOrderByCriterion(orderBy);

  std::string orderDirection;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_ORDER_DIRECTION, orderDirection) )
    searchCrit.setOrderDirection(orderDirection);

  std::string address;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_ADDRESS, address) )
    searchCrit.setAbonentAddress(address);

  std::string msgLimit;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_MSG_LIMIT, msgLimit) ) {
    size_t msgLimitAsNumber;
    msgLimitAsNumber = strtol(msgLimit.c_str(), (char**)NULL, 10);
    if ( !msgLimitAsNumber && errno ) 
      error("selectTaskMessages", ARGUMENT_MSG_LIMIT);
    searchCrit.setMsgLimit(msgLimitAsNumber);
  }

  Variant result(StringListType);
  Array<std::string> taskMessagesList = admin.getTaskMessages(atoi(taskId.c_str()), searchCrit);
  for (int i=0; i<taskMessagesList.Count(); i++)
    result.appendValueToStringList(taskMessagesList[i].c_str());
  return result;
}

Variant InfoSmeComponent::selectTasksStatistic(const Arguments& args)
{
  /*InfoSme_Tasks_Stat_SearchCriterion searchCrit;

  std::string taskId;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_NAME_ID, taskId ) )
    searchCrit.setTaskId(taskId);

  std::string startPeriodAsString;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_START_PERIOD, startPeriodAsString) ) {
    searchCrit.setStartPeriod(atoi(startPeriodAsString.c_str()));
  }

  std::string endPeriodAsString;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_END_PERIOD, endPeriodAsString) ) {
    searchCrit.setEndPeriod(atoi(endPeriodAsString.c_str()));
  }

  Variant result(StringListType);
  Array<std::string> tasksStatList = admin.getTasksStatistic(searchCrit);
  for (int i=0; i<tasksStatList.Count(); i++)
    result.appendValueToStringList(tasksStatList[i].c_str());*/

  Variant result(StringListType);
  result.appendValueToStringList("not implemented");
  return result;
}

void InfoSmeComponent::endDeliveryMessagesGeneration(const Arguments& args)
{
    if (!args.Exists(ARGUMENT_NAME_ID)) 
        error("endDeliveryMessagesGeneration", ARGUMENT_NAME_ID);
    Variant arg = args[ARGUMENT_NAME_ID];
    const char* id = (arg.getType() == StringType) ? arg.getStringValue():0;
    if (!id || id[0] == '\0')
        error("endDeliveryMessagesGeneration", ARGUMENT_NAME_ID);
    
    try { admin.endDeliveryMessagesGeneration(atoi(id));
    } catch (Exception& exc) {
        throw AdminException("Failed to add task '%s'. Cause: %s", id, exc.what());
    } catch (std::exception& exc) {
        throw AdminException("Failed to add task '%s'. Cause: %s", id, exc.what());
    } catch (...) {
        throw AdminException("Failed to add task '%s'. Cause is unknown", id);
    }
}

void InfoSmeComponent::changeDeliveryTextMessage(const Arguments& args)
{
  std::string taskId;
  if ( !getParameterIfExistsAndNotNull(args, ARGUMENT_NAME_ID, taskId) )
    error("changeDeliveryTextMessage", ARGUMENT_NAME_ID);

  std::string newTextMsg;
  if ( !getParameterIfExistsAndNotNull(args, ARGUMENT_NEW_TEXT_MESSAGE, newTextMsg) )
    error("changeDeliveryTextMessage", ARGUMENT_NEW_TEXT_MESSAGE);

  smsc_log_debug(logger, "InfoSmeComponent::changeDeliveryTextMessage::: taskId=[%s]",taskId.c_str());

  std::string recordId;
  /*  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_RECORD_ID, recordId) )
    admin.deleteDeliveryMessageByRecordId(taskId, recordId);
    else {*/
  InfoSme_T_SearchCriterion searchCrit;

  std::string stateAsString;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_STATE, stateAsString) ) {
    searchCrit.setState((uint8_t)atoi(stateAsString.c_str()));
  }
  std::string fromDate;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_FROM_DATE, fromDate) ) {
    time_t fromDateAsUnixTime;
    if (!convertFullDateFormatToUnixTime(fromDate, &fromDateAsUnixTime))
      error("changeDeliveryTextMessage", ARGUMENT_FROM_DATE);

    searchCrit.setFromDate(fromDateAsUnixTime);
  }
  std::string toDate;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_TO_DATE, toDate ) ) {
    time_t toDateAsUnixTime;
    if (!convertFullDateFormatToUnixTime(toDate, &toDateAsUnixTime))
      error("changeDeliveryTextMessage", ARGUMENT_TO_DATE);

    searchCrit.setToDate(toDateAsUnixTime);
  }
  std::string address;
  if ( getParameterIfExistsAndNotNull(args, ARGUMENT_ADDRESS, address) )
    searchCrit.setAbonentAddress(address);

  admin.changeDeliveryTextMessageByCompositCriterion(atoi(taskId.c_str()), newTextMsg, searchCrit);
  //}
}

void InfoSmeComponent::applyRetryPolicies(const Arguments& args)
{
  admin.applyRetryPolicies();
}


Variant InfoSmeComponent::sendSms(const Arguments& args)
{
  using namespace smsc::smpp;
  std::string org=args["source"].getStringValue();
  std::string dst=args["destination"].getStringValue();
  std::string txt=args["message"].getStringValue();
  bool isFlash=args["flash"].getBooleanValue();
  
  return Variant((long)admin.sendSms(org,dst,txt,isFlash));
}

void InfoSmeComponent::reloadSmscAndRegions(const Arguments& args)
{
    admin.reloadSmscAndRegions();
}

}}

