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
  Component infoSmeComponent = null;
  Method method_startTaskProcessor = null;
  Method method_stopTaskProcessor = null;
  Method method_isTaskProcessorRuning = null;
  Method method_startTaskScheduler = null;
  Method method_stopTaskScheduler = null;
  Method method_isTaskSchedulerRuning = null;
  Method method_addTasks = null;
  Method method_removeTasks = null;
  Method method_setTaskEnabled = null;
  Method method_startTasks = null;
  Method method_stopTasks = null;
  Method method_addSchedule = null;
  Method method_removeSchedule = null;
  Method method_changeSchedule = null;
  Method method_flushStatistics = null;


  public InfoSmeTransport(ServiceInfo info) throws AdminException
  {
    super(info);
  }

  public synchronized void startTaskProcessor() throws AdminException
  {
    call(infoSmeComponent, method_startTaskProcessor, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized void stopTaskProcessor() throws AdminException
  {
    call(infoSmeComponent, method_stopTaskProcessor, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized boolean isTaskProcessorRuning() throws AdminException
  {
    Object result = call(infoSmeComponent, method_isTaskProcessorRuning, Type.Types[Type.BooleanType], new HashMap());
    if (result instanceof Boolean)
      return ((Boolean) result).booleanValue();
    else
      throw new AdminException("isTaskProcessorRuning: Incorrect return type \"" + result.getClass().getName() + "\"");
  }

  public synchronized void startTaskScheduler() throws AdminException
  {
    call(infoSmeComponent, method_startTaskScheduler, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized void stopTaskScheduler() throws AdminException
  {
    call(infoSmeComponent, method_stopTaskScheduler, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized boolean isTaskSchedulerRuning() throws AdminException
  {
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
  public synchronized void addTasks(List taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("taskIds", taskIds);
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
    params.put("taskIds", taskIds);
    call(infoSmeComponent, method_removeTasks, Type.Types[Type.StringType], params);
  }

  public synchronized void setTaskEnabled(String taskId, boolean enabled) throws AdminException
  {
    Map params = new HashMap();
    params.put("taskId", taskId);
    params.put("enabled", new Boolean(enabled));
    call(infoSmeComponent, method_setTaskEnabled, Type.Types[Type.StringType], params);
  }

  public synchronized void startTasks(List taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("taskIds", taskIds);
    call(infoSmeComponent, method_startTasks, Type.Types[Type.StringType], params);
  }

  public synchronized void stopTasks(List taskIds) throws AdminException
  {
    Map params = new HashMap();
    params.put("taskIds", taskIds);
    call(infoSmeComponent, method_stopTasks, Type.Types[Type.StringType], params);
  }

  public synchronized void addSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap();
    params.put("scheduleId", scheduleId);
    call(infoSmeComponent, method_addSchedule, Type.Types[Type.StringType], params);
  }

  public synchronized void removeSchedule(String scheduleId) throws AdminException
  {
    Map params = new HashMap();
    params.put("scheduleId", scheduleId);
    call(infoSmeComponent, method_removeSchedule, Type.Types[Type.StringType], params);
  }

  public synchronized void changeSchedule(String oldScheduleId, String newScheduleId) throws AdminException
  {
    Map params = new HashMap();
    params.put("oldScheduleId", oldScheduleId);
    params.put("newScheduleId", newScheduleId);
    call(infoSmeComponent, method_changeSchedule, Type.Types[Type.StringType], params);
  }

  public synchronized void flushStatistics() throws AdminException
  {
    call(infoSmeComponent, method_flushStatistics, Type.Types[Type.StringType], new HashMap());
  }


  protected void checkComponents()
  {
    super.checkComponents();
    if (infoSmeComponent == null
            || method_startTaskProcessor == null || method_stopTaskProcessor == null || method_isTaskProcessorRuning == null
            || method_startTaskScheduler == null || method_stopTaskScheduler == null || method_isTaskSchedulerRuning == null
            || method_addTasks == null || method_removeTasks == null || method_setTaskEnabled == null || method_startTasks == null || method_stopTasks == null
            || method_addSchedule == null || method_removeSchedule == null || method_changeSchedule == null
            || method_flushStatistics == null) {

      infoSmeComponent = (Component) getInfo().getComponents().get("InfoSme");

      method_startTaskProcessor = (Method) infoSmeComponent.getMethods().get("startTaskProcessor");
      method_stopTaskProcessor = (Method) infoSmeComponent.getMethods().get("stopTaskProcessor");
      method_isTaskProcessorRuning = (Method) infoSmeComponent.getMethods().get("isTaskProcessorRuning");

      method_startTaskScheduler = (Method) infoSmeComponent.getMethods().get("startTaskScheduler");
      method_stopTaskScheduler = (Method) infoSmeComponent.getMethods().get("stopTaskScheduler");
      method_isTaskSchedulerRuning = (Method) infoSmeComponent.getMethods().get("isTaskSchedulerRuning");

      method_addTasks = (Method) infoSmeComponent.getMethods().get("addTasks");
      method_removeTasks = (Method) infoSmeComponent.getMethods().get("removeTasks");
      method_setTaskEnabled = (Method) infoSmeComponent.getMethods().get("setTaskEnabled");
      method_startTasks = (Method) infoSmeComponent.getMethods().get("startTasks");
      method_stopTasks = (Method) infoSmeComponent.getMethods().get("stopTasks");

      method_addSchedule = (Method) infoSmeComponent.getMethods().get("addSchedule");
      method_removeSchedule = (Method) infoSmeComponent.getMethods().get("removeSchedule");
      method_changeSchedule = (Method) infoSmeComponent.getMethods().get("changeSchedule");

      method_flushStatistics = (Method) infoSmeComponent.getMethods().get("flushStatistics");
    }
  }
}
