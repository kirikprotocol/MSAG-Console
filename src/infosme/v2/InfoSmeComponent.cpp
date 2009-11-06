#include "InfoSmeComponent.h"

namespace smsc {
namespace infosme2 {

using namespace smsc::admin::service;

InfoSmeComponent::InfoSmeComponent(InfoSmeAdmin& admin) :
log_(smsc::logger::Logger::getInstance("is2.compon")),
admin_(admin)
{
    Parameters empty_params;
    Method start_task_processor(unsigned(startTaskProcessorMethod),
                                "startTaskProcessor", empty_params, StringType);
    Method stop_task_processor(unsigned(stopTaskProcessorMethod),
                               "stopTaskProcessor", empty_params, StringType);
    Method is_task_processor_running(unsigned(isTaskProcessorRunningMethod),
                                     "isTaskProcessorRunning", empty_params, BooleanType);
    Method start_task_scheduler(unsigned(startTaskSchedulerMethod),
                                "startTaskScheduler", empty_params, StringType);
    Method stop_task_scheduler(unsigned(stopTaskSchedulerMethod),
                               "stopTaskScheduler", empty_params, StringType);
    Method is_task_scheduler_running(unsigned(isTaskSchedulerRunningMethod),
                                     "isTaskSchedulerRunning", empty_params, BooleanType);
    /*
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
     */

    methods_[start_task_processor.getName()]         = start_task_processor;
    methods_[stop_task_processor.getName()]          = stop_task_processor;
    methods_[is_task_processor_running.getName()]    = is_task_processor_running;
    methods_[start_task_scheduler.getName()]         = start_task_scheduler;
    methods_[stop_task_scheduler.getName()]          = stop_task_scheduler;
    methods_[is_task_scheduler_running.getName()]    = is_task_scheduler_running;

    /*
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
     */
}


InfoSmeComponent::~InfoSmeComponent()
{}


Variant InfoSmeComponent::call( const Method& method, const Arguments& args ) throw (AdminException)
{
    try {
        switch ( method.getId() ) {
        case startTaskProcessorMethod:
            admin_.startTaskProcessor(); 
            break;
        case stopTaskProcessorMethod:
            admin_.stopTaskProcessor(); 
            break;
        case isTaskProcessorRunningMethod:
            return Variant(admin_.isTaskProcessorRunning());
        case startTaskSchedulerMethod:
            admin_.startTaskScheduler(); 
            break;
        case stopTaskSchedulerMethod:
            admin_.stopTaskScheduler();
            break;
        case isTaskSchedulerRunningMethod:
            return Variant(admin_.isTaskSchedulerRunning());
        default:
            smsc_log_debug(log_,"unknown method '%s' id=%u", method.getName(), method.getId() );
            throw AdminException("unknown method '%s'", method.getName());
        }
        return Variant("");

    } catch ( AdminException& e ) {
        smsc_log_error(log_,"AdminException: %s", e.what());
        throw;
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"Exception: %s", e.what());
        throw AdminException("%s",e.what());
    } catch (...) {
        smsc_log_error(log_,"Unknown Exception");
        throw AdminException("Unknown exception in call(%s)",method.getName());
    }
}

}
}
