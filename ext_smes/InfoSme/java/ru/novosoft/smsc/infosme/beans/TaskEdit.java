package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.Transliterator;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 2:31:58 PM
 */
public class TaskEdit extends InfoSmeBean
{
  private String mbDone = null;
  private String mbCancel = null;

  private boolean transliterate = false;
  private boolean initialized = false;
  private boolean create = false;
  private String oldTask = null;
  private String oldTaskName = null;

  private Task task = new Task();

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      if (!create) {
        if (getId() == null || getId().length() == 0)
          return error("infosme.error.task_id_undefined");
        oldTask = getId();

        try {
          task = new Task(getConfig(), getId());
          oldTaskName = getName();
        } catch (Exception e) {
          logger.error(e);
          return error("infosme.error.config_param", e.getMessage());
        }
      } else {
        task.setPriority(1);
        task.setMessagesCacheSize(100);
        task.setMessagesCacheSleep(1);
        task.setUncommitedInGeneration(1);
        task.setUncommitedInProcess(1);
      }
    }
    if (oldTask == null) oldTask = "";
    if (oldTaskName == null) oldTaskName = "";

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    if (mbDone != null)      return done();
    if (mbCancel != null)    return RESULT_DONE;

    return result;
  }

  protected int done()
  {
    if (getId() == null || getId().length() == 0)
      return error("infosme.error.task_id_undefined");
    if (!create) { // Edit task
        if (!oldTaskName.equals(getName()) && task.isContainsInConfigByName(getConfig()))
          return error("Task with name='"+getName()+"' already exists. Please specify another name");
        if (!oldTask.equals(getId())) {
          if (task.isContainsInConfig(getConfig()))
            return error("Task with id='"+getId()+"' already exists. Please specify another id");
          Task.removeTaskFromConfig(getConfig(), oldTask);
        }
    } else { // Create new task
        if (task.isContainsInConfig(getConfig()))
          return error("Task with id='"+getId()+"' already exists. Please specify another id");
        if (task.isContainsInConfigByName(getConfig()))
          return error("Task with name='"+getName()+"' already exists. Please specify another name");
    }
    if (transliterate) task.setTemplate(Transliterator.translit(task.getTemplate()));
    task.storeToConfig(getConfig());
    getInfoSmeContext().setChangedTasks(true);
    return RESULT_DONE;
  }

  public Collection getAllProviders() {
    return new SortedList(getConfig().getSectionChildShortSectionNames("InfoSme.DataProvider"));
  }

  public boolean isInitialized() {
    return initialized;
  }
  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public boolean isCreate() {
    return create;
  }
  public void setCreate(boolean create) {
    this.create = create;
  }

  public String getOldTask() {
    return oldTask;
  }
  public void setOldTask(String oldTask) {
    this.oldTask = oldTask;
  }
  public String getOldTaskName() {
    return oldTaskName;
  }
  public void setOldTaskName(String oldTaskName) {
    this.oldTaskName = oldTaskName;
  }

  public String getId() {
    return task.getId();
  }
  public void setId(String sectionName) {
    this.task.setId(sectionName);
  }

  public String getAddress() {
    return task.getAddress();
  }
  public void setAddress(String address) {
    this.task.setAddress(address);
  }
  public String getProvider() {
    return task.getProvider();
  }
  public void setProvider(String provider) {
    this.task.setProvider(provider);
  }

  public boolean isEnabled() {
    return task.isEnabled();
  }
  public void setEnabled(boolean enabled) {
    this.task.setEnabled(enabled);
  }
  public boolean isDelivery() {
    return task.isDelivery();
  }
  public void setDelivery(boolean delivery) {
    this.task.setDelivery(delivery);
  }

  public int getPriorityInt() {
    return task.getPriority();
  }
  public void setPriorityInt(int priority) {
    this.task.setPriority(priority);
  }
  public String getPriority() {
    return String.valueOf(task.getPriority());
  }
  public void setPriority(String priority)
  {
    try {
      this.task.setPriority(Integer.decode(priority).intValue());
    } catch (Throwable e) {
      logger.error("Couldn't set priority to value \"" + priority + "\"", e);
    }
  }

  public boolean isRetryOnFail() {
    return task.isRetryOnFail();
  }
  public void setRetryOnFail(boolean retryOnFail) {
    this.task.setRetryOnFail(retryOnFail);
  }

  public boolean isReplaceMessage() {
    return task.isReplaceMessage();
  }
  public void setReplaceMessage(boolean replaceMessage) {
    this.task.setReplaceMessage(replaceMessage);
  }

  public String getSvcType() {
    return task.getSvcType();
  }
  public void setSvcType(String svcType) {
    this.task.setSvcType(svcType);
  }

  public String getEndDate() {
    return task.getEndDate();
  }
  public void setEndDate(String endDate) {
    this.task.setEndDate(endDate);
  }

  public String getRetryTime() {
    return task.getRetryTime();
  }
  public void setRetryTime(String retryTime) {
    this.task.setRetryTime(retryTime);
  }

  public String getValidityPeriod() {
    return task.getValidityPeriod();
  }
  public void setValidityPeriod(String validityPeriod) {
    this.task.setValidityPeriod(validityPeriod);
  }

  public String getValidityDate() {
    return task.getValidityDate();
  }
  public void setValidityDate(String validityDate) {
    this.task.setValidityDate(validityDate);
  }

  public String getActivePeriodStart() {
    return task.getActivePeriodStart();
  }
  public void setActivePeriodStart(String activePeriodStart) {
    this.task.setActivePeriodStart(activePeriodStart);
  }

  public String getActivePeriodEnd() {
    return task.getActivePeriodEnd();
  }
  public void setActivePeriodEnd(String activePeriodEnd) {
    this.task.setActivePeriodEnd(activePeriodEnd);
  }

  public String[] getActiveWeekDays() {
    return (String[])(this.task.getActiveWeekDays().toArray());
  }
  public void setActiveWeekDays(String[] activeWeekDays) {
    Collection awd = new ArrayList();
    for(int i=0; i<activeWeekDays.length; i++) awd.add(activeWeekDays[i]);
    this.task.setActiveWeekDays(awd);
  }
  public boolean isWeekDayActive(String weekday) {
    return this.task.isWeekDayActive(weekday);
  }
  public String getActiveWeekDaysString()
  {
    String str = "";
    Collection awd = this.task.getActiveWeekDays();
    int total = (awd == null) ? 0:awd.size();
    if (total > 0) {
      int added=0;
      if (task.isWeekDayActive("Mon")) { str += "Monday";    if (++added < total) str += ", "; }
      if (task.isWeekDayActive("Tue")) { str += "Tuesday";   if (++added < total) str += ", "; }
      if (task.isWeekDayActive("Wed")) { str += "Wednesday"; if (++added < total) str += ", "; }
      if (task.isWeekDayActive("Thu")) { str += "Thursday";  if (++added < total) str += ", "; }
      if (task.isWeekDayActive("Fri")) { str += "Friday";    if (++added < total) str += ", "; }
      if (task.isWeekDayActive("Sat")) { str += "Saturday";  if (++added < total) str += ", "; }
      if (task.isWeekDayActive("Sun"))   str += "Sunday";
    }
    return str;
  }

  public String getQuery() {
    return task.getQuery();
  }
  public void setQuery(String query) {
    this.task.setQuery(query);
  }

  public String getTemplate() {
    return task.getTemplate();
  }
  public void setTemplate(String template) {
    this.task.setTemplate(template);
  }

  public String getMbDone() {
    return mbDone;
  }
  public void setMbDone(String mbDone) {
    this.mbDone = mbDone;
  }

  public String getMbCancel() {
    return mbCancel;
  }
  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public String getName() {
    return task.getName();
  }
  public void setName(String name) {
    this.task.setName(name);
  }

  public int getDsTimeoutInt() {
    return task.getDsTimeout();
  }
  public void setDsTimeoutInt(int dsTimeout) {
    this.task.setDsTimeout(dsTimeout);
  }
  public String getDsTimeout() {
    return String.valueOf(task.getDsTimeout());
  }
  public void setDsTimeout(String dsTimeout)
  {
    try {
      this.task.setDsTimeout(Integer.decode(dsTimeout).intValue());
    } catch (Throwable e) {
      logger.error("Couldn't set dsOwnTimeout to value \"" + dsTimeout + "\"", e);
    }
  }

  public int getMessagesCacheSizeInt() {
    return task.getMessagesCacheSize();
  }
  public void setMessagesCacheSizeInt(int messagesCacheSize) {
    this.task.setMessagesCacheSize(messagesCacheSize);
  }

  public int getMessagesCacheSleepInt() {
    return task.getMessagesCacheSleep();
  }
  public void setMessagesCacheSleepInt(int messagesCacheSleep) {
    this.task.setMessagesCacheSleep(messagesCacheSleep);
  }

  public boolean isTransactionMode() {
    return task.isTransactionMode();
  }
  public void setTransactionMode(boolean transactionMode) {
    this.task.setTransactionMode(transactionMode);
  }

  public int getUncommitedInGenerationInt() {
    return task.getUncommitedInGeneration();
  }
  public void setUncommitedInGenerationInt(int uncommitedInGeneration) {
    this.task.setUncommitedInGeneration(uncommitedInGeneration);
  }

  public int getUncommitedInProcessInt() {
    return task.getUncommitedInProcess();
  }
  public void setUncommitedInProcessInt(int uncommitedInProcess) {
    this.task.setUncommitedInProcess(uncommitedInProcess);
  }

  public String getMessagesCacheSize() {
    return String.valueOf(task.getMessagesCacheSize());
  }
  public void setMessagesCacheSize(String messagesCacheSize)
  {
    try {
      this.task.setMessagesCacheSize(Integer.decode(messagesCacheSize).intValue());
    } catch (Throwable e) {
      logger.error("Couldn't set messagesCacheSize to value \"" + messagesCacheSize + "\"", e);
    }
  }

  public String getMessagesCacheSleep() {
    return String.valueOf(task.getMessagesCacheSleep());
  }
  public void setMessagesCacheSleep(String messagesCacheSleep)
  {
    try {
      this.task.setMessagesCacheSleep(Integer.decode(messagesCacheSleep).intValue());
    } catch (Throwable e) {
      logger.error("Couldn't set messagesCacheSleep to value \"" + messagesCacheSleep + "\"", e);
    }
  }

  public String getUncommitedInGeneration() {
    return String.valueOf(task.getUncommitedInGeneration());
  }
  public void setUncommitedInGeneration(String uncommitedInGeneration)
  {
    try {
      this.task.setUncommitedInGeneration(Integer.decode(uncommitedInGeneration).intValue());
    } catch (Throwable e) {
      logger.error("Couldn't set uncommitedInGeneration to value \"" + uncommitedInGeneration + "\"", e);
    }
  }

  public String getUncommitedInProcess() {
    return String.valueOf(task.getUncommitedInProcess());
  }
  public void setUncommitedInProcess(String uncommitedInProcess)
  {
    try {
      this.task.setUncommitedInProcess(Integer.decode(uncommitedInProcess).intValue());
    } catch (Throwable e) {
      logger.error("Couldn't set uncommitedInGeneration to value \"" + uncommitedInProcess + "\"", e);
    }
  }

  public boolean isTrackIntegrity() {
    return task.isTrackIntegrity();
  }
  public void setTrackIntegrity(boolean trackIntegrity) {
    task.setTrackIntegrity(trackIntegrity);
  }

  public boolean isKeepHistory() {
    return task.isKeepHistory();
  }
  public void setKeepHistory(boolean keepHistory) {
    task.setKeepHistory(keepHistory);
  }

  public boolean isTransliterate() {
    return transliterate;
  }
  public void setTransliterate(boolean transliterate) {
    this.transliterate = transliterate;
  }
}
