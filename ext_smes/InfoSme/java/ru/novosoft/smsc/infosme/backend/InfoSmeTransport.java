package ru.novosoft.smsc.infosme.backend;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.*;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 24.09.2003
 * Time: 17:21:16
 */
class InfoSmeTransport extends Service
{
  private static final String SME_COMPONENT_ID = "InfoSme";
  private static final String method_startTaskProcessor_ID = "startTaskProcessor";
  private static final String method_stopTaskProcessor_ID = "stopTaskProcessor";
  private static final String method_isTaskProcessorRuning_ID = "isTaskProcessorRunning";

  private static final String method_startTaskScheduler_ID = "startTaskScheduler";
  private static final String method_stopTaskScheduler_ID = "stopTaskScheduler";
  private static final String method_isTaskSchedulerRuning_ID = "isTaskSchedulerRunning";

  private static final String method_addTask_ID = "addTask";
  private static final String method_removeTask_ID = "removeTask";
  private static final String method_changeTask_ID = "changeTask";

  private static final String method_isTaskEnabled_ID = "isTaskEnabled";
  private static final String method_setTaskEnabled_ID = "setTaskEnabled";

  private static final String method_startTasks_ID = "startTasks";
  private static final String method_stopTasks_ID = "stopTasks";
  private static final String method_getGeneratingTasksMethod_ID = "getGeneratingTasksMethod";
  private static final String method_getProcessingTasksMethod_ID = "getProcessingTasksMethod";

  private static final String method_addSchedule_ID = "addSchedule";
  private static final String method_removeSchedule_ID = "removeSchedule";
  private static final String method_changeSchedule_ID = "changeSchedule";

  private static final String method_flushStatistics_ID = "flushStatistics";


  public InfoSmeTransport(ServiceInfo info, int port) throws AdminException
  {
    super(info, port);
  }

  public synchronized void startTaskProcessor() throws AdminException
  {
    call(SME_COMPONENT_ID, method_startTaskProcessor_ID, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized void stopTaskProcessor() throws AdminException
  {
    call(SME_COMPONENT_ID, method_stopTaskProcessor_ID, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized boolean isTaskProcessorRuning() throws AdminException
  {
    Object result = call(SME_COMPONENT_ID, method_isTaskProcessorRuning_ID, Type.Types[Type.BooleanType], new HashMap());
    if (result instanceof Boolean)
      return ((Boolean) result).booleanValue();
    else
      throw new AdminException("isTaskProcessorRuning: Incorrect return type \"" + result.getClass().getName() + "\"");
  }

  public synchronized void startTaskScheduler() throws AdminException
  {
    call(SME_COMPONENT_ID, method_startTaskScheduler_ID, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized void stopTaskScheduler() throws AdminException
  {
    call(SME_COMPONENT_ID, method_stopTaskScheduler_ID, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized boolean isTaskSchedulerRuning() throws AdminException
  {
    Object result = call(SME_COMPONENT_ID, method_isTaskSchedulerRuning_ID, Type.Types[Type.BooleanType], new HashMap());
    if (result instanceof Boolean)
      return ((Boolean) result).booleanValue();
    else
      throw new AdminException("isTaskSchedulerRuning: Incorrect return type \"" + result.getClass().getName() + "\"");
  }

  /**
   * add all tasks
   *
   * @param taskId - таска, которую надо добавить (она уже должна быть прописана в конфиге)
   * @throws AdminException - если что-то не получилось
   */
  public synchronized void addTask(String taskId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", taskId);
    call(SME_COMPONENT_ID, method_addTask_ID, Type.Types[Type.StringType], params);
  }

  /**
   * remove tasks
   *
   * @param taskId - таска, которую надо удалить
   * @throws AdminException - если что-то не получилось
   */
  public synchronized void removeTask(String taskId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", taskId);
    call(SME_COMPONENT_ID, method_removeTask_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void changeTask(String taskId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", taskId);
    call(SME_COMPONENT_ID, method_changeTask_ID, Type.Types[Type.StringType], params);
  }

  public synchronized boolean isTaskEnabled(String taskId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", taskId);
    Object result = call(SME_COMPONENT_ID, method_isTaskEnabled_ID, Type.Types[Type.BooleanType], params);
    if (result instanceof Boolean)
      return ((Boolean) result).booleanValue();
    else
      throw new AdminException("isTaskProcessorRuning: Incorrect return type \"" + result.getClass().getName() + "\"");
  }

  public synchronized void setTaskEnabled(String taskId, boolean enabled) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", taskId);
    params.put("enabled", new Boolean(enabled));
    call(SME_COMPONENT_ID, method_setTaskEnabled_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void startTasks(Collection taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", taskIds);
    call(SME_COMPONENT_ID, method_startTasks_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void stopTasks(Collection taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", taskIds);
    call(SME_COMPONENT_ID, method_stopTasks_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void addSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", scheduleId);
    call(SME_COMPONENT_ID, method_addSchedule_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void removeSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", scheduleId);
    call(SME_COMPONENT_ID, method_removeSchedule_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void changeSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", scheduleId);
    call(SME_COMPONENT_ID, method_changeSchedule_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void flushStatistics() throws AdminException
  {
    call(SME_COMPONENT_ID, method_flushStatistics_ID, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized List getGeneratingTasks() throws AdminException
  {
    return (List) call(SME_COMPONENT_ID, method_getGeneratingTasksMethod_ID, Type.Types[Type.StringListType], new HashMap());
  }

  public synchronized List getProcessingTasks() throws AdminException
  {
    return (List) call(SME_COMPONENT_ID, method_getProcessingTasksMethod_ID, Type.Types[Type.StringListType], new HashMap());
  }
}
