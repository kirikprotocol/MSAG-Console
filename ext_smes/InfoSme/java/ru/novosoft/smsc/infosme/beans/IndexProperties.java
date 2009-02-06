package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskFilter;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 02.10.2003
 * Time: 17:16:54
 */
public abstract class IndexProperties extends TasksListBean
{
  private InfoSme infoSme;
  private boolean infosmeStarted;
  private boolean taskProcessorRuning = false;
  private boolean taskSchedulerRuning = false;
  private String mbApply = null;
  private String mbReset = null;
  private String mbStart = null;
  private String mbStop = null;
  private String mbEnableTask = null;
  private String mbDisableTask = null;
  private String mbStartTask = null;
  private String mbStopTask = null;

  private String[] apply = new String[0];
  private Set applySet = new HashSet();
  private String[] toStart = new String[0];
  private Set toStartSet = new HashSet();

  protected IndexProperties(TaskFilter tasksFilter) {
    super(tasksFilter);
  }

  protected IndexProperties() {
    super();
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    infoSme = getInfoSmeContext().getInfoSme();

    try {
      getAppContext().getHostsManager().refreshServices();
      infoSme.updateInfo(appContext);
    } catch (AdminException e) {
      logger.error("Could not refresh Info SME status", e);
      result = error("infosme.error.refresh_status", e);
    }

    infosmeStarted = infoSme.getInfo().isOnline();
    try {
      if (infosmeStarted) {
        taskProcessorRuning = infoSme.isTaskProcessorRuning();
        taskSchedulerRuning = infoSme.isTaskSchedulerRuning();
      } else {
        //message("Info SME is not running");
      }
    } catch (AdminException e) {
      logger.error("Could not get task processor or task scheduler status", e);
      result = error("infosme.error.get_tp_ts_status", e);
    }

    applySet.addAll(Arrays.asList(apply));
    toStartSet.addAll(Arrays.asList(toStart));

    if (mbApply != null)
      return apply(request);
    if (mbReset != null)
      return reset(request);
    if (mbStart != null)
      return start();
    if (mbStop != null)
      return stop();
    if (mbEnableTask != null)
      return enableTask();
    if (mbDisableTask != null)
      return disableTask();
    if (mbStartTask != null)
      return startTask();
    if (mbStopTask != null)
      return stopTask();

    return result;
  }

  protected abstract int apply(HttpServletRequest req);

  protected abstract int reset(HttpServletRequest req);

  protected abstract int start();

  protected abstract int stop();

  protected abstract int enableTask();

  protected abstract int disableTask();

  protected abstract int startTask();

  protected abstract int stopTask();

  public String getMbApply()
  {
    return mbApply;
  }

  public void setMbApply(String mbApply)
  {
    this.mbApply = mbApply;
  }

  public String getMbReset()
  {
    return mbReset;
  }

  public void setMbReset(String mbReset)
  {
    this.mbReset = mbReset;
  }

  public boolean isChangedAll()
  {
    return getInfoSmeConfig().isOptionsModified() || getInfoSmeConfig().isDriversChanged() || getInfoSmeConfig().isProvidersChanged();
//    return getInfoSmeContext().isChangedOptions() || getInfoSmeContext().isChangedDrivers() || getInfoSmeContext().isChangedProviders();
  }

  public boolean isChangedRetryPolicies() {
//    return getInfoSmeContext().isChangedRetryPolicies();
    return getInfoSmeConfig().isRetryPoliciesChanged();
  }

  public boolean isChangedTasks(HttpServletRequest req)
  {
//    return getInfoSmeContext().isChangedTasks();
    String user = isUserAdmin(req) ? null : req.getRemoteUser();
    return getInfoSmeConfig().isTasksChanged(user);
  }

  public boolean isChangedShedules()
  {
//    return getInfoSmeContext().isChangedSchedules();
    return getInfoSmeConfig().isSchedulesChanged();
  }


  public String[] getApply()
  {
    return apply;
  }

  public void setApply(String[] apply)
  {
    this.apply = apply;
  }

  public boolean isTaskProcessorRuning()
  {
    return taskProcessorRuning;
  }

  public boolean isTaskSchedulerRuning()
  {
    return taskSchedulerRuning;
  }

  public boolean isInfosmeStarted()
  {
    return infosmeStarted;
  }

  public String[] getToStart()
  {
    return toStart;
  }

  public void setToStart(String[] toStart)
  {
    this.toStart = toStart;
  }

  public boolean isApply(String applyId)
  {
    return applySet.contains(applyId);
  }

  public boolean isToStart(String toStartId)
  {
    return toStartSet.contains(toStartId);
  }

  public String getMbStart()
  {
    return mbStart;
  }

  public void setMbStart(String mbStart)
  {
    this.mbStart = mbStart;
  }

  public String getMbStop()
  {
    return mbStop;
  }

  public void setMbStop(String mbStop)
  {
    this.mbStop = mbStop;
  }

  public String getMbEnableTask()
  {
    return mbEnableTask;
  }

  public void setMbEnableTask(String mbEnableTask)
  {
    this.mbEnableTask = mbEnableTask;
  }

  public String getMbDisableTask()
  {
    return mbDisableTask;
  }

  public void setMbDisableTask(String mbDisableTask)
  {
    this.mbDisableTask = mbDisableTask;
  }

  public String getMbStartTask()
  {
    return mbStartTask;
  }

  public void setMbStartTask(String mbStartTask)
  {
    this.mbStartTask = mbStartTask;
  }

  public String getMbStopTask()
  {
    return mbStopTask;
  }

  public void setMbStopTask(String mbStopTask)
  {
    this.mbStopTask = mbStopTask;
  }
}
