package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.security.Principal;
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

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    infoSme = getInfoSmeContext().getInfoSme();

    try {
      getAppContext().getHostsManager().refreshServices();
      infoSme.updateInfo(appContext);
    } catch (AdminException e) {
      logger.error("Could not refresh Info SME status", e);
      return error("Could not refresh Info SME status", e);
    }

    infosmeStarted = infoSme.getInfo().getStatus() == ServiceInfo.STATUS_RUNNING;
    try {
      if (infosmeStarted) {
        taskProcessorRuning = infoSme.isTaskProcessorRuning();
        taskSchedulerRuning = infoSme.isTaskSchedulerRuning();
      } else {
        //message("Info SME is not running");
      }
    } catch (AdminException e) {
      logger.error("Could not get task processor or task scheduler status", e);
      return error("Could not get task processor or task scheduler status", e);
    }

    applySet.addAll(Arrays.asList(apply));
    toStartSet.addAll(Arrays.asList(toStart));

    if (mbApply != null)
      return apply();
    if (mbReset != null)
      return reset();
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

  protected abstract int apply();

  protected abstract int reset();

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
    return getInfoSmeContext().isChangedOptions() || getInfoSmeContext().isChangedDrivers() || getInfoSmeContext().isChangedProviders();
  }

  public boolean isChangedTasks()
  {
    return getInfoSmeContext().isChangedTasks();
  }

  public boolean isChangedShedules()
  {
    return getInfoSmeContext().isChangedSchedules();
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

  protected InfoSme getInfoSme()
  {
    return infoSme;
  }
}
