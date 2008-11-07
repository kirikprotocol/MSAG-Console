package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataItem;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataSource;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyQuery;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.Transliterator;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

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

  private String taskId = "";
  private String name = "";
  private String address = "";
  private String provider = "";
  private boolean enabled = false;
  private boolean delivery = false;
  private int priority = 0;
  private boolean retryOnFail = false;
  private boolean replaceMessage = false;
  private String svcType = "";
  private String endDate = "";
  private String startDate = "";
  private String validityPeriod = "";
  private String validityDate = "";
  private String activePeriodStart = "";
  private String activePeriodEnd = "";
  private String[] activeWeekDays = new String[0];
  private String query = "";
  private String template = "";
  private int dsTimeout = 0;
  private int messagesCacheSize = 0;
  private int messagesCacheSleep = 0;
  private boolean transactionMode = false;
  private int uncommitedInGeneration = 0;
  private int uncommitedInProcess = 0;
  private boolean trackIntegrity = false;
  private boolean keepHistory = false;
  private boolean flash = false;
  private String retryPolicy = "";

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {

      try {
        Task task = getTask();
        oldTaskName = task.getName();
        oldTask = task.getId();
        taskToPage(task);
      } catch (Exception e) {
        logger.error(e);
        return error("infosme.error.config_param", e.getMessage());
      }
    }
    if (oldTask == null) oldTask = "";
    if (oldTaskName == null) oldTaskName = "";

    return result;
  }

  private Task getTask() throws Config.WrongParamTypeException, Config.ParamNotFoundException, AdminException {
    if (!create)
      return new Task(getConfig(), getId());
    else {
      Task task = getInfoSmeContext().getTaskManager().createTask();
      task.setPriority(1);
      task.setMessagesCacheSize(100);
      task.setMessagesCacheSleep(1);
      task.setUncommitedInGeneration(1);
      task.setUncommitedInProcess(1);
      return task;
    }
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    if (mbDone != null) result = done();
    else if (mbCancel != null) result = RESULT_DONE;

    return result;
  }

  private void taskToPage(Task task) {
    taskId = task.getId();
    name = task.getName();
    address = task.getAddress();
    provider = task.getProvider();
    enabled = task.isEnabled();
    delivery = task.isDelivery();
    priority = task.getPriority();
    retryOnFail = task.isRetryOnFail();
    replaceMessage = task.isReplaceMessage();
    svcType = task.getSvcType();
    endDate = task.getEndDate();
    startDate = task.getStartDate();
    validityPeriod = task.getValidityPeriod();
    validityDate = task.getValidityDate();
    activePeriodStart = task.getActivePeriodStart();
    activePeriodEnd = task.getActivePeriodEnd();
    activeWeekDays = new String[task.getActiveWeekDays().size()];
    int i=0;
    for (Iterator iter = task.getActiveWeekDays().iterator(); iter.hasNext();) {
      activeWeekDays[i] = (String)iter.next();
      i++;
    }
    query = task.getQuery();
    template = task.getTemplate();
    dsTimeout = task.getDsTimeout();
    messagesCacheSize = task.getMessagesCacheSize();
    messagesCacheSleep = task.getMessagesCacheSleep();
    transactionMode = task.isTransactionMode();
    uncommitedInGeneration = task.getUncommitedInGeneration();
    uncommitedInProcess = task.getUncommitedInProcess();
    trackIntegrity = task.isTrackIntegrity();
    keepHistory = task.isKeepHistory();
    flash = task.isFlash();
    retryPolicy = task.getRetryPolicy();
  }

  private void pageToTask(Task task) {
    task.setName(name);
    task.setAddress(address);
    task.setProvider(provider);
    task.setEnabled(enabled);
    task.setDelivery(delivery);
    task.setPriority(priority);
    task.setRetryOnFail(retryOnFail);
    task.setReplaceMessage(replaceMessage);
    task.setSvcType(svcType);
    task.setEndDate(endDate);
    task.setStartDate(startDate);
    task.setValidityPeriod(validityPeriod);
    task.setValidityDate(validityDate);
    task.setActivePeriodStart(activePeriodStart);
    task.setActivePeriodEnd(activePeriodEnd);
    Collection awd = new ArrayList();
    for(int i=0; i<activeWeekDays.length; i++) awd.add(activeWeekDays[i]);
    task.setActiveWeekDays(awd);
    task.setQuery(query);
    task.setTemplate(template);
    task.setDsTimeout(dsTimeout);
    task.setMessagesCacheSize(messagesCacheSize);
    task.setMessagesCacheSleep(messagesCacheSleep);
    task.setTransactionMode(transactionMode);

    task.setUncommitedInGeneration(uncommitedInGeneration);
    task.setUncommitedInProcess(uncommitedInProcess);
    task.setTrackIntegrity(trackIntegrity);
    task.setKeepHistory(keepHistory);
    task.setFlash(flash);
    task.setRetryPolicy(retryPolicy);
  }

  protected int done()
  {
    if (getId() == null || getId().length() == 0)
      return error("infosme.error.task_id_undefined");
    final Task task = new Task(getId());
    pageToTask(task);
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
    return taskId;
  }
  public void setId(String id) {
    this.taskId = id;
  }

  public String getAddress() {
    return address;
  }
  public void setAddress(String address) {
    this.address = address;
  }
  public String getProvider() {
    return provider;
  }
  public void setProvider(String provider) {
    this.provider = provider;
  }

  public boolean isEnabled() {
    return enabled;
  }
  public void setEnabled(boolean enabled) {
    this.enabled = enabled;
  }
  public boolean isDelivery() {
    return delivery;
  }
  public void setDelivery(boolean delivery) {
    this.delivery = delivery;
  }


  public String getPriority() {
    return String.valueOf(priority);
  }

  public void setPriority(String priority) {
    try {
      this.priority = Integer.decode(priority).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set priority to value \"" + priority + "\"", e);
    }
  }

  public boolean isRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(boolean retryOnFail) {
    this.retryOnFail = retryOnFail;
  }

  public boolean isReplaceMessage() {
    return replaceMessage;
  }

  public void setReplaceMessage(boolean replaceMessage) {
    this.replaceMessage = replaceMessage;
  }

  public String getSvcType() {
    return svcType;
  }

  public void setSvcType(String svcType) {
    this.svcType = svcType;
  }

  public String getEndDate() {
    return endDate;
  }

  public void setEndDate(String endDate) {
    this.endDate = endDate;
  }

  public String getStartDate() {
    return startDate;
  }

  public void setStartDate(String startDate) {
    this.startDate = startDate;
  }

  public List getRetryPolicies() throws AdminException {
    QueryResultSet rs = new RetryPolicyDataSource().query(getConfig(), new RetryPolicyQuery(1000, "name", 0));
    List result = new ArrayList(rs.size() + 1);
    for (int i=0; i<rs.size(); i++) {
      RetryPolicyDataItem item = (RetryPolicyDataItem)rs.get(i);
      result.add(item.getName());
    }
    return result;
  }

  public String getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(String validityPeriod) {
    this.validityPeriod = validityPeriod;
  }

  public String getValidityDate() {
    return validityDate;
  }

  public void setValidityDate(String validityDate) {
    this.validityDate = validityDate;
  }

  public String getActivePeriodStart() {
    return activePeriodStart;
  }
  public void setActivePeriodStart(String activePeriodStart) {
    this.activePeriodStart = activePeriodStart;
  }

  public String getActivePeriodEnd() {
    return activePeriodEnd;
  }

  public void setActivePeriodEnd(String activePeriodEnd) {
    this.activePeriodEnd = activePeriodEnd;
  }

  public String[] getActiveWeekDays() {
    return activeWeekDays;
  }
  public void setActiveWeekDays(String[] activeWeekDays) {
    this.activeWeekDays = activeWeekDays;
  }

  public boolean isWeekDayActive(String weekday) {
    for(int i=0; i<activeWeekDays.length; i++)
      if (activeWeekDays[i].equals(weekday))
        return true;
    return false;
  }

  public String getActiveWeekDaysString()
  {
    String str = "";
    int total = activeWeekDays.length;
    if (total > 0) {
      int added=0;
      if (isWeekDayActive("Mon")) { str += "Monday";    if (++added < total) str += ", "; }
      if (isWeekDayActive("Tue")) { str += "Tuesday";   if (++added < total) str += ", "; }
      if (isWeekDayActive("Wed")) { str += "Wednesday"; if (++added < total) str += ", "; }
      if (isWeekDayActive("Thu")) { str += "Thursday";  if (++added < total) str += ", "; }
      if (isWeekDayActive("Fri")) { str += "Friday";    if (++added < total) str += ", "; }
      if (isWeekDayActive("Sat")) { str += "Saturday";  if (++added < total) str += ", "; }
      if (isWeekDayActive("Sun"))   str += "Sunday";
    }
    return str;
  }

  public String getQuery() {
    return query;
  }

  public void setQuery(String query) {
    this.query = query;
  }

  public String getTemplate() {
    return template;
  }

  public void setTemplate(String template) {
    this.template = template;
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
    return name;
  }
  public void setName(String name) {
    this.name = name;
  }

  public int getDsTimeoutInt() {
    return dsTimeout;
  }
  public void setDsTimeoutInt(int dsTimeout) {
    this.dsTimeout = dsTimeout;
  }

  public String getDsTimeout() {
    return String.valueOf(dsTimeout);
  }

  public void setDsTimeout(String dsTimeout)
  {
    try {
      this.dsTimeout = Integer.decode(dsTimeout).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set dsOwnTimeout to value \"" + dsTimeout + "\"", e);
    }
  }

  public int getMessagesCacheSizeInt() {
    return messagesCacheSize;
  }
  public void setMessagesCacheSizeInt(int messagesCacheSize) {
    this.messagesCacheSize = messagesCacheSize;
  }

  public int getMessagesCacheSleepInt() {
    return messagesCacheSleep;
  }
  public void setMessagesCacheSleepInt(int messagesCacheSleep) {
    this.messagesCacheSleep = messagesCacheSleep;
  }

  public boolean isTransactionMode() {
    return transactionMode;
  }
  public void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
  }

  public int getUncommitedInGenerationInt() {
    return uncommitedInGeneration;
  }
  public void setUncommitedInGenerationInt(int uncommitedInGeneration) {
    this.uncommitedInGeneration = uncommitedInGeneration;
  }

  public int getUncommitedInProcessInt() {
    return uncommitedInProcess;
  }
  public void setUncommitedInProcessInt(int uncommitedInProcess) {
    this.uncommitedInProcess = uncommitedInProcess;
  }

  public String getMessagesCacheSize() {
    return String.valueOf(messagesCacheSize);
  }
  public void setMessagesCacheSize(String messagesCacheSize)
  {
    try {
      this.messagesCacheSize = Integer.decode(messagesCacheSize).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set messagesCacheSize to value \"" + messagesCacheSize + "\"", e);
    }
  }

  public String getMessagesCacheSleep() {
    return String.valueOf(messagesCacheSleep);
  }
  public void setMessagesCacheSleep(String messagesCacheSleep)
  {
    try {
      this.messagesCacheSleep = Integer.decode(messagesCacheSleep).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set messagesCacheSleep to value \"" + messagesCacheSleep + "\"", e);
    }
  }

  public String getUncommitedInGeneration() {
    return String.valueOf(uncommitedInGeneration);
  }
  public void setUncommitedInGeneration(String uncommitedInGeneration)
  {
    try {
      this.uncommitedInGeneration = Integer.decode(uncommitedInGeneration).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set uncommitedInGeneration to value \"" + uncommitedInGeneration + "\"", e);
    }
  }

  public String getUncommitedInProcess() {
    return String.valueOf(uncommitedInProcess);
  }
  public void setUncommitedInProcess(String uncommitedInProcess)
  {
    try {
      this.uncommitedInProcess = Integer.decode(uncommitedInProcess).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set uncommitedInGeneration to value \"" + uncommitedInProcess + "\"", e);
    }
  }

  public boolean isTrackIntegrity() {
    return trackIntegrity;
  }

  public void setTrackIntegrity(boolean trackIntegrity) {
    this.trackIntegrity = trackIntegrity;
  }

  public boolean isKeepHistory() {
    return keepHistory;
  }

  public void setKeepHistory(boolean keepHistory) {
    this.keepHistory = keepHistory;
  }

  public boolean isTransliterate() {
    return transliterate;
  }

  public void setTransliterate(boolean transliterate) {
    this.transliterate = transliterate;
  }

  public boolean isFlash() {
    return flash;
  }

  public void setFlash(boolean flash) {
    this.flash = flash;
  }

  public String getRetryPolicy() {
    return retryPolicy;
  }

  public void setRetryPolicy(String retryPolicy) {
    this.retryPolicy = retryPolicy;
  }
}
