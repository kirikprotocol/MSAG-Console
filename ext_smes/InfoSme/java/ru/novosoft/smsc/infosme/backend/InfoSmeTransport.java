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
  private Component infoSmeComponent = null;
  private Method method_startTaskProcessor = null;
  private Method method_stopTaskProcessor = null;
  private Method method_isTaskProcessorRuning = null;
  private Method method_startTaskScheduler = null;
  private Method method_stopTaskScheduler = null;
  private Method method_isTaskSchedulerRuning = null;

  private Method method_addTask = null;
  private Method method_removeTask = null;
  private Method method_changeTask = null;

  private Method method_isTaskEnabled = null;
  private Method method_setTaskEnabled = null;

  private Method method_startTasks = null;
  private Method method_stopTasks = null;
  private Method method_getProcessingTasksMethod = null;
  private Method method_getGeneratingTasksMethod = null;

  private Method method_addSchedule = null;
  private Method method_removeSchedule = null;
  private Method method_changeSchedule = null;

  private Method method_flushStatistics = null;


  public InfoSmeTransport(ServiceInfo info) throws AdminException
  {
    super(info);
  }

  public synchronized void startTaskProcessor() throws AdminException
  {
    refreshComponents();
    call(infoSmeComponent, method_startTaskProcessor, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized void stopTaskProcessor() throws AdminException
  {
    refreshComponents();
    call(infoSmeComponent, method_stopTaskProcessor, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized boolean isTaskProcessorRuning() throws AdminException
  {
    refreshComponents();
    Object result = call(infoSmeComponent, method_isTaskProcessorRuning, Type.Types[Type.BooleanType], new HashMap());
    if (result instanceof Boolean)
      return ((Boolean) result).booleanValue();
    else
      throw new AdminException("isTaskProcessorRuning: Incorrect return type \"" + result.getClass().getName() + "\"");
  }

  public synchronized void startTaskScheduler() throws AdminException
  {
    refreshComponents();
    call(infoSmeComponent, method_startTaskScheduler, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized void stopTaskScheduler() throws AdminException
  {
    refreshComponents();
    call(infoSmeComponent, method_stopTaskScheduler, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized boolean isTaskSchedulerRuning() throws AdminException
  {
    refreshComponents();
    Object result = call(infoSmeComponent, method_isTaskSchedulerRuning, Type.Types[Type.BooleanType], new HashMap());
    if (result instanceof Boolean)
      return ((Boolean) result).booleanValue();
    else
      throw new AdminException("isTaskSchedulerRuning: Incorrect return type \"" + result.getClass().getName() + "\"");
  }

  /**
   * add all tasks
   * @param taskId - таска, которую надо добавить (она уже должна быть прописана в конфиге)
   * @throws AdminException - если что-то не получилось
   */
  public synchronized void addTask(String  taskId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", taskId);
    refreshComponents();
    call(infoSmeComponent, method_addTask, Type.Types[Type.StringType], params);
  }

  /**
   * remove tasks
   * @param taskId - таска, которую надо удалить
   * @throws AdminException - если что-то не получилось
   */
  public synchronized void removeTask(String taskId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", taskId);
    refreshComponents();
    call(infoSmeComponent, method_removeTask, Type.Types[Type.StringType], params);
  }

  public synchronized void changeTask(String taskId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", taskId);
    refreshComponents();
    call(infoSmeComponent, method_changeTask, Type.Types[Type.StringType], params);
  }

  public synchronized boolean isTaskEnabled(String taskId) throws AdminException
  {
    refreshComponents();
    Map params = new HashMap();
    params.put("id", taskId);
    Object result = call(infoSmeComponent, method_isTaskEnabled, Type.Types[Type.BooleanType], params);
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
    refreshComponents();
    call(infoSmeComponent, method_setTaskEnabled, Type.Types[Type.StringType], params);
  }

  public synchronized void startTasks(Collection taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", taskIds);
    refreshComponents();
    call(infoSmeComponent, method_startTasks, Type.Types[Type.StringType], params);
  }

  public synchronized void stopTasks(Collection taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", taskIds);
    refreshComponents();
    call(infoSmeComponent, method_stopTasks, Type.Types[Type.StringType], params);
  }

  public synchronized void addSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", scheduleId);
    refreshComponents();
    call(infoSmeComponent, method_addSchedule, Type.Types[Type.StringType], params);
  }

  public synchronized void removeSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", scheduleId);
    refreshComponents();
    call(infoSmeComponent, method_removeSchedule, Type.Types[Type.StringType], params);
  }

  public synchronized void changeSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap();
    params.put("id", scheduleId);
    refreshComponents();
    call(infoSmeComponent, method_changeSchedule, Type.Types[Type.StringType], params);
  }

  public synchronized void flushStatistics() throws AdminException
  {
    refreshComponents();
    call(infoSmeComponent, method_flushStatistics, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized List getGeneratingTasks() throws AdminException
  {
    refreshComponents();
    return (List) call(infoSmeComponent, method_getGeneratingTasksMethod, Type.Types[Type.StringListType], new HashMap());
  }

  public synchronized List getProcessingTasks() throws AdminException
  {
    refreshComponents();
    return (List) call(infoSmeComponent, method_getProcessingTasksMethod, Type.Types[Type.StringListType], new HashMap());
  }

  protected void checkComponents()
  {
    super.checkComponents();
    if (infoSmeComponent == null
            || method_startTaskProcessor == null || method_stopTaskProcessor == null || method_isTaskProcessorRuning == null
            || method_startTaskScheduler == null || method_stopTaskScheduler == null || method_isTaskSchedulerRuning == null
            || method_addTask == null || method_removeTask == null || method_changeTask == null
            || method_isTaskEnabled == null || method_setTaskEnabled == null
            || method_startTasks == null || method_stopTasks == null
            || method_getGeneratingTasksMethod == null || method_getProcessingTasksMethod == null
            || method_addSchedule == null || method_removeSchedule == null || method_changeSchedule == null
            || method_flushStatistics == null) {

      infoSmeComponent = (Component) getInfo().getComponents().get("InfoSme");

      method_startTaskProcessor = (Method) infoSmeComponent.getMethods().get("startTaskProcessor");
      method_stopTaskProcessor = (Method) infoSmeComponent.getMethods().get("stopTaskProcessor");
      method_isTaskProcessorRuning = (Method) infoSmeComponent.getMethods().get("isTaskProcessorRunning");

      method_startTaskScheduler = (Method) infoSmeComponent.getMethods().get("startTaskScheduler");
      method_stopTaskScheduler = (Method) infoSmeComponent.getMethods().get("stopTaskScheduler");
      method_isTaskSchedulerRuning = (Method) infoSmeComponent.getMethods().get("isTaskSchedulerRunning");

      method_addTask = (Method) infoSmeComponent.getMethods().get("addTask");
      method_removeTask = (Method) infoSmeComponent.getMethods().get("removeTask");
      method_changeTask = (Method) infoSmeComponent.getMethods().get("changeTask");

      method_isTaskEnabled = (Method) infoSmeComponent.getMethods().get("isTaskEnabled");
      method_setTaskEnabled = (Method) infoSmeComponent.getMethods().get("setTaskEnabled");

      method_startTasks = (Method) infoSmeComponent.getMethods().get("startTasks");
      method_stopTasks = (Method) infoSmeComponent.getMethods().get("stopTasks");
      method_getGeneratingTasksMethod = (Method) infoSmeComponent.getMethods().get("getGeneratingTasksMethod");
      method_getProcessingTasksMethod = (Method) infoSmeComponent.getMethods().get("getProcessingTasksMethod");

      method_addSchedule = (Method) infoSmeComponent.getMethods().get("addSchedule");
      method_removeSchedule = (Method) infoSmeComponent.getMethods().get("removeSchedule");
      method_changeSchedule = (Method) infoSmeComponent.getMethods().get("changeSchedule");

      method_flushStatistics = (Method) infoSmeComponent.getMethods().get("flushStatistics");
    }
  }
}
