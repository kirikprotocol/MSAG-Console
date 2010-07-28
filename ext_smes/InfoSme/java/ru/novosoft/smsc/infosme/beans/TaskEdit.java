package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.infosme.backend.config.provider.Provider;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataItem;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataSource;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyQuery;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.user.UserDataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.user.UserQuery;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.Transliterator;

import javax.servlet.http.HttpServletRequest;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 2:31:58 PM
 */
public class TaskEdit extends InfoSmeBean {

  private final static SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
  private static final SimpleDateFormat tf = new SimpleDateFormat("HH:mm:ss");

  public static final int RESULT_MESSAGES = InfoSmeBean.PRIVATE_RESULT + 1;
  public static final int RESULT_STATISTICS = InfoSmeBean.PRIVATE_RESULT + 2;
  protected static final int PRIVATE_RESULT = InfoSmeBean.PRIVATE_RESULT + 3;


  private String mbDone = null;
  private String mbCancel = null;
  private String mbMessages = null;
  private String mbStatistics = null;

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
  private boolean useDataSm = false;
  private int uncommitedInGeneration = 0;
  private int uncommitedInProcess = 0;
  private boolean trackIntegrity = false;
  private boolean keepHistory = false;
  private boolean flash = false;
  private String retryPolicy = "";
  private boolean secret;
  private boolean secretFlash;
  private String secretMessage;
  private String owner;
  private int deliveryMode = Task.DELIVERY_MODE_SMS;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    return result;
  }

  private Task getTask(User user) throws AdminException {
    if (!create)
      return getInfoSmeContext().getInfoSmeConfig().getTask(getId()).cloneTask();
    else {
      return getInfoSmeContext().getInfoSmeConfig().createTask(user);
    }
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    User user = getUser(request);
    if (!initialized) {
      try {
        Task task = getTask(getUser(request));
        oldTaskName = task.getName();
        oldTask = task.getId();
        taskToPage(task, user);
      } catch (Exception e) {
        logger.error(e);
        return error("infosme.error.config_param", e.getMessage());
      }
    }
    if (oldTask == null) oldTask = "";
    if (oldTaskName == null) oldTaskName = "";

    if (mbDone != null) try {
      result = done(user);
    } catch (AdminException e) {
      return error(e.getMessage(), e);
    }
    else if (mbCancel != null) result = RESULT_DONE;
    else if (mbMessages != null) result = RESULT_MESSAGES;
    else if (mbStatistics != null) result = RESULT_STATISTICS;

    return result;
  }

  private void taskToPage(Task task, User user) {
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
    endDate = task.getEndDate() == null ? "" : df.format(user.getUserTime(task.getEndDate()));
    startDate = task.getStartDate() == null ? "" : df.format(user.getUserTime(task.getStartDate()));
    validityPeriod = task.getValidityPeriod() == null ? "" : String.valueOf(task.getValidityPeriod());
    validityDate = task.getValidityDate() == null ? "" : df.format(user.getUserTime(task.getValidityDate()));
    activePeriodStart = task.getActivePeriodStart() == null ? "" : tf.format(user.getUserTime(task.getActivePeriodStart()));
    activePeriodEnd = task.getActivePeriodEnd() == null ? "" : tf.format(user.getUserTime(task.getActivePeriodEnd()));
    activeWeekDays = new String[task.getActiveWeekDays().size()];
    int i = 0;
    for (Iterator iter = task.getActiveWeekDays().iterator(); iter.hasNext();) {
      activeWeekDays[i] = (String) iter.next();
      i++;
    }
    query = task.getQuery();
    template = task.getTemplate();
    dsTimeout = task.getDsTimeout();
    messagesCacheSize = task.getMessagesCacheSize();
    messagesCacheSleep = task.getMessagesCacheSleep();
    transactionMode = task.isTransactionMode();
    useDataSm = task.isUseDataSm();
    deliveryMode = task.getDeliveryMode();
    uncommitedInGeneration = task.getUncommitedInGeneration();
    uncommitedInProcess = task.getUncommitedInProcess();
    trackIntegrity = task.isTrackIntegrity();
    keepHistory = task.isKeepHistory();
    flash = task.isFlash();
    retryPolicy = task.getRetryPolicy();
    secret = task.isSecret();
    secretFlash = task.isSecretFlash();
    secretMessage = task.getSecretMessage();
    owner = task.getOwner();
  }

  private void pageToTask(Task task, User user) throws AdminException {
    task.setName(name);
    task.setAddress(address);
    task.setEnabled(enabled);
    task.setTransactionMode(transactionMode);
    task.setUseDataSm(useDataSm);
    task.setPriority(priority);
    try {
      task.setEndDate(endDate.trim().length() == 0 ? null : user.getLocalTime(df.parse(endDate)));
      task.setStartDate(startDate.trim().length() == 0 ? null : user.getLocalTime(df.parse(startDate)));
      task.setActivePeriodStart(activePeriodStart.trim().length() == 0 ? null : user.getLocalTime(tf.parse(activePeriodStart)));
      task.setActivePeriodEnd(activePeriodEnd.trim().length() == 0 ? null : user.getLocalTime(tf.parse(activePeriodEnd)));
    } catch (Throwable e) {
      e.printStackTrace();
      throw new AdminException("Invalid field format", e);
    }
    task.setActiveWeekDays(Arrays.asList(activeWeekDays));

    task.setDelivery(delivery);
    task.setQuery(query);
    task.setTemplate(template);
    task.setFlash(flash);

    if (isUserAdmin(user)) {
      task.setProvider(provider);
      task.setRetryOnFail(retryOnFail);
      task.setReplaceMessage(replaceMessage);
      task.setSvcType(svcType);
      task.setTrackIntegrity(trackIntegrity);
      task.setKeepHistory(keepHistory);
      task.setDsTimeout(dsTimeout);
      task.setMessagesCacheSize(messagesCacheSize);
      task.setMessagesCacheSleep(messagesCacheSleep);
      task.setUncommitedInGeneration(uncommitedInGeneration);
      task.setUncommitedInProcess(uncommitedInProcess);
      try {
        task.setValidityPeriod((validityPeriod == null || validityPeriod.trim().length() == 0) ? null : Integer.valueOf(validityPeriod));
        task.setValidityDate(validityDate.trim().length() == 0 ? null : user.getLocalTime(df.parse(validityDate)));
      } catch (Throwable e) {
        throw new AdminException("Invalid validity date/period format");
      }
      task.setRetryPolicy(retryPolicy);
      task.setOwner(owner);

    }
    // NOTE: ussd push is set the last to override other settings
    task.setDeliveryMode(deliveryMode);
  }

  protected int done(User user) throws AdminException {
    if (getId() == null || getId().length() == 0)
      return error("infosme.error.task_id_undefined");
    final Task task = getTask(user);
    pageToTask(task, user);
    if (!create) { // Edit task
      if (!oldTaskName.equals(getName()) && getInfoSmeContext().getInfoSmeConfig().containsTaskWithName(task.getName()))
        return error("Task with name='" + getName() + "' already exists. Please specify another name");
    } else { // Create new task
      if (getInfoSmeContext().getInfoSmeConfig().containsTaskWithId(task.getId()))
        return error("Task with id='" + getId() + "' already exists. Please specify another id");
      if (getInfoSmeContext().getInfoSmeConfig().containsTaskWithName(task.getName()))
        return error("Task with name='" + getName() + "' already exists. Please specify another name");
    }
    if (transliterate) task.setTemplate(Transliterator.translit(task.getTemplate()));
    getInfoSmeContext().getInfoSmeConfig().addTask(task);
    return RESULT_DONE;
  }

  public Collection getUsers() {
    QueryResultSet users = appContext.getUserManager().query(new UserQuery(100, preferences.getUserFilter(), preferences.getUsersSortOrder(), 0));
    ArrayList result = new ArrayList(10);
    for (int i = 0; i < users.size(); i++) {
      UserDataItem uitem = (UserDataItem) users.get(i);
      User user = appContext.getUserManager().getUser(uitem.getLogin());
      if (user.getRoles().contains(INFOSME_ADMIN_ROLE) || user.getRoles().contains(INFOSME_MARKET_ROLE))
        result.add(user.getLogin());
    }
    return result;
  }

  public Collection getAllProviders() {
    List names = new ArrayList();
    for (Iterator iter = getInfoSmeConfig().getProviders().iterator(); iter.hasNext();)
      names.add(((Provider) iter.next()).getName());
    return new SortedList(names);
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
    for (int i = 0; i < rs.size(); i++) {
      RetryPolicyDataItem item = (RetryPolicyDataItem) rs.get(i);
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
    for (int i = 0; i < activeWeekDays.length; i++)
      if (activeWeekDays[i].equals(weekday))
        return true;
    return false;
  }

  public String getActiveWeekDaysString() {
    String str = "";
    int total = activeWeekDays.length;
    if (total > 0) {
      int added = 0;
      if (isWeekDayActive("Mon")) {
        str += "Monday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Tue")) {
        str += "Tuesday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Wed")) {
        str += "Wednesday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Thu")) {
        str += "Thursday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Fri")) {
        str += "Friday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Sat")) {
        str += "Saturday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Sun")) str += "Sunday";
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

  public void setDsTimeout(String dsTimeout) {
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

  public boolean isUseDataSm() {
    return useDataSm;
  }

  public void setUseDataSm(boolean useDataSm) {
    this.useDataSm = useDataSm;
  }

  public boolean isUssdPushAllowed() {
    return getInfoSmeConfig().getUssdPushFeature() != null && getInfoSmeConfig().getUssdPushFeature().booleanValue();
  }

  public int getDeliveryMode() {
    return deliveryMode;
  }

  public void setDeliveryMode(int deliveryMode) {
    this.deliveryMode = deliveryMode;
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

  public void setMessagesCacheSize(String messagesCacheSize) {
    try {
      this.messagesCacheSize = Integer.decode(messagesCacheSize).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set messagesCacheSize to value \"" + messagesCacheSize + "\"", e);
    }
  }

  public String getMessagesCacheSleep() {
    return String.valueOf(messagesCacheSleep);
  }

  public void setMessagesCacheSleep(String messagesCacheSleep) {
    try {
      this.messagesCacheSleep = Integer.decode(messagesCacheSleep).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set messagesCacheSleep to value \"" + messagesCacheSleep + "\"", e);
    }
  }

  public String getUncommitedInGeneration() {
    return String.valueOf(uncommitedInGeneration);
  }

  public void setUncommitedInGeneration(String uncommitedInGeneration) {
    try {
      this.uncommitedInGeneration = Integer.decode(uncommitedInGeneration).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set uncommitedInGeneration to value \"" + uncommitedInGeneration + "\"", e);
    }
  }

  public String getUncommitedInProcess() {
    return String.valueOf(uncommitedInProcess);
  }

  public void setUncommitedInProcess(String uncommitedInProcess) {
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

  public boolean isSecret() {
    return secret;
  }

  public boolean isSecretFlash() {
    return secretFlash;
  }

  public String getSecretMessage() {
    return secretMessage;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner;
  }

  public String getMbMessages() {
    return mbMessages;
  }

  public void setMbMessages(String mbMessages) {
    this.mbMessages = mbMessages;
  }

  public String getMbStatistics() {
    return mbStatistics;
  }

  public void setMbStatistics(String mbStatistics) {
    this.mbStatistics = mbStatistics;
  }
}
