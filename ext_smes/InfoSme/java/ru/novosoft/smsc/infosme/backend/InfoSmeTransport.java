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

  private Method method_addTasks = null;
  private Method method_removeTasks = null;
  private Method method_changeTasks = null;

  private Method method_isTaskEnabled = null;
  private Method method_setTaskEnabled = null;

  private Method method_startTasks = null;
  private Method method_stopTasks = null;
  private Method method_getProcessingTasksMethod = null;
  private Method method_getGeneratingTasksMethod = null;

  private Method method_addSchedules = null;
  private Method method_removeSchedules = null;
  private Method method_changeSchedules = null;

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
   * @param taskIds - список тасок, которые надо добавить (они уже должны быть прописаны в конфиге)
   * @throws AdminException - если что-то не получилось
   */
  public synchronized void addTasks(Collection taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", taskIds);
    refreshComponents();
    call(infoSmeComponent, method_addTasks, Type.Types[Type.StringType], params);
  }

  /**
   * remove tasks
   * @param taskIds - список тасок, которые надо удалить
   * @throws AdminException - если что-то не получилось
   */
  public synchronized void removeTasks(Collection taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", taskIds);
    refreshComponents();
    call(infoSmeComponent, method_removeTasks, Type.Types[Type.StringType], params);
  }

  public synchronized void changeTasks(Collection taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", taskIds);
    refreshComponents();
    call(infoSmeComponent, method_changeTasks, Type.Types[Type.StringType], params);
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

  public synchronized void addSchedules(Collection scheduleIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", scheduleIds);
    refreshComponents();
    call(infoSmeComponent, method_addSchedules, Type.Types[Type.StringType], params);
  }

  public synchronized void removeSchedules(Collection scheduleIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", scheduleIds);
    refreshComponents();
    call(infoSmeComponent, method_removeSchedules, Type.Types[Type.StringType], params);
  }

  public synchronized void changeSchedules(Collection scheduleIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("ids", scheduleIds);
    refreshComponents();
    call(infoSmeComponent, method_changeSchedules, Type.Types[Type.StringType], params);
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
            || method_addTasks == null || method_removeTasks == null || method_changeTasks == null
            || method_isTaskEnabled == null || method_setTaskEnabled == null
            || method_startTasks == null || method_stopTasks == null
            || method_getGeneratingTasksMethod == null || method_getProcessingTasksMethod == null
            || method_addSchedules == null || method_removeSchedules == null || method_changeSchedules == null
            || method_flushStatistics == null) {

      infoSmeComponent = (Component) getInfo().getComponents().get("InfoSme");

      method_startTaskProcessor = (Method) infoSmeComponent.getMethods().get("startTaskProcessor");
      method_stopTaskProcessor = (Method) infoSmeComponent.getMethods().get("stopTaskProcessor");
      method_isTaskProcessorRuning = (Method) infoSmeComponent.getMethods().get("isTaskProcessorRunning");

      method_startTaskScheduler = (Method) infoSmeComponent.getMethods().get("startTaskScheduler");
      method_stopTaskScheduler = (Method) infoSmeComponent.getMethods().get("stopTaskScheduler");
      method_isTaskSchedulerRuning = (Method) infoSmeComponent.getMethods().get("isTaskSchedulerRunning");

      method_addTasks = (Method) infoSmeComponent.getMethods().get("addTasks");
      method_removeTasks = (Method) infoSmeComponent.getMethods().get("removeTasks");
      method_changeTasks = (Method) infoSmeComponent.getMethods().get("changeTasks");

      method_isTaskEnabled = (Method) infoSmeComponent.getMethods().get("isTaskEnabled");
      method_setTaskEnabled = (Method) infoSmeComponent.getMethods().get("setTaskEnabled");

      method_startTasks = (Method) infoSmeComponent.getMethods().get("startTasks");
      method_stopTasks = (Method) infoSmeComponent.getMethods().get("stopTasks");
      method_getGeneratingTasksMethod = (Method) infoSmeComponent.getMethods().get("getGeneratingTasksMethod");
      method_getProcessingTasksMethod = (Method) infoSmeComponent.getMethods().get("getProcessingTasksMethod");

      method_addSchedules = (Method) infoSmeComponent.getMethods().get("addSchedules");
      method_removeSchedules = (Method) infoSmeComponent.getMethods().get("removeSchedules");
      method_changeSchedules = (Method) infoSmeComponent.getMethods().get("changeSchedules");

      method_flushStatistics = (Method) infoSmeComponent.getMethods().get("flushStatistics");
    }
  }
}
