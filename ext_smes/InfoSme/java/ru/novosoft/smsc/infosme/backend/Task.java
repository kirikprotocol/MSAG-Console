package ru.novosoft.smsc.infosme.backend;

import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 03.10.2003
 * Time: 17:10:35
 */
public class Task
{
  private String id = "";
  private String name = "";
  private String provider = "";
  private boolean enabled = false;
  private int priority = 0;
  private boolean retryOnFail = false;
  private boolean replaceMessage = false;
  private String svcType = "";
  private String endDate = "";
  private String retryTime = "";
  private String validityPeriod = "";
  private String validityDate = "";
  private String activePeriodStart = "";
  private String activePeriodEnd = "";
  private String query = "";
  private String template = "";
  private int dsTimeout = 0;
  private int messagesCacheSize = 0;
  private int messagesCacheSleep = 0;
  private boolean transactionMode = false;
  private boolean keepHistory = false;
  private int uncommitedInGeneration = 0;
  private int uncommitedInProcess = 0;
  private boolean trackIntegrity;

  public Task()
  {
  }

  public Task(String id, String name, String provider, boolean enabled, int priority, boolean retryOnFail, boolean replaceMessage, String svcType, String endDate, String retryTime, String validityPeriod, String validityDate, String activePeriodStart, String activePeriodEnd, String query, String template, int dsTimeout,
              int messagesCacheSize, int messagesCacheSleep, boolean transactionMode,
              int uncommitedInGeneration, int uncommitedInProcess, boolean trackIntegrity, boolean keepHistory)
  {
    this();
    this.id = id;
    this.name = name;
    this.provider = provider;
    this.enabled = enabled;
    this.priority = priority;
    this.retryOnFail = retryOnFail;
    this.replaceMessage = replaceMessage;
    this.svcType = svcType;
    this.endDate = endDate;
    this.retryTime = retryTime;
    this.validityPeriod = validityPeriod;
    this.validityDate = validityDate;
    this.activePeriodStart = activePeriodStart;
    this.activePeriodEnd = activePeriodEnd;
    this.query = query;
    this.template = template;
    this.dsTimeout = dsTimeout;
    this.messagesCacheSize = messagesCacheSize;
    this.messagesCacheSleep = messagesCacheSleep;
    this.transactionMode = transactionMode;
    this.uncommitedInGeneration = uncommitedInGeneration;
    this.uncommitedInProcess = uncommitedInProcess;
    this.trackIntegrity = trackIntegrity;
    this.keepHistory = keepHistory;
  }

  public Task(Config config, String id) throws Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    this();
    final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(id);
    this.id = id;
    name = config.getString(prefix + ".name");
    provider = config.getString(prefix + ".dsId");
    enabled = config.getBool(prefix + ".enabled");
    priority = config.getInt(prefix + ".priority");
    retryOnFail = config.getBool(prefix + ".retryOnFail");
    replaceMessage = config.getBool(prefix + ".replaceMessage");
    svcType = config.getString(prefix + ".svcType");
    endDate = config.getString(prefix + ".endDate");
    retryTime = config.getString(prefix + ".retryTime");
    validityPeriod = config.getString(prefix + ".validityPeriod");
    validityDate = config.getString(prefix + ".validityDate");
    activePeriodStart = config.getString(prefix + ".activePeriodStart");
    activePeriodEnd = config.getString(prefix + ".activePeriodEnd");
    query = config.getString(prefix + ".query");
    template = config.getString(prefix + ".template");
    dsTimeout = config.getInt(prefix + ".dsTimeout");
    messagesCacheSize = config.getInt(prefix + ".messagesCacheSize");
    messagesCacheSleep = config.getInt(prefix + ".messagesCacheSleep");
    transactionMode = config.getBool(prefix + ".transactionMode");
    uncommitedInGeneration = config.getInt(prefix + ".uncommitedInGeneration");
    uncommitedInProcess = config.getInt(prefix + ".uncommitedInProcess");
    trackIntegrity = config.getBool(prefix + ".trackIntegrity");
    keepHistory = config.getBool(prefix + ".keepHistory");
  }

  public void storeToConfig(Config config)
  {
    final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(id);
    config.setString(prefix + ".name", name);
    config.setString(prefix + ".dsId", provider);
    config.setBool(prefix + ".enabled", enabled);
    config.setInt(prefix + ".priority", priority);
    config.setBool(prefix + ".retryOnFail", retryOnFail);
    config.setBool(prefix + ".replaceMessage", replaceMessage);
    config.setString(prefix + ".svcType", svcType);
    config.setString(prefix + ".endDate", endDate);
    config.setString(prefix + ".retryTime", retryTime);
    config.setString(prefix + ".validityPeriod", validityPeriod);
    config.setString(prefix + ".validityDate", validityDate);
    config.setString(prefix + ".activePeriodStart", activePeriodStart);
    config.setString(prefix + ".activePeriodEnd", activePeriodEnd);
    config.setString(prefix + ".query", query);
    config.setString(prefix + ".template", template);
    config.setInt(prefix + ".dsTimeout", dsTimeout);
    config.setInt(prefix + ".messagesCacheSize", messagesCacheSize);
    config.setInt(prefix + ".messagesCacheSleep", messagesCacheSleep);
    config.setBool(prefix + ".transactionMode", transactionMode);
    config.setInt(prefix + ".uncommitedInGeneration", uncommitedInGeneration);
    config.setInt(prefix + ".uncommitedInProcess", uncommitedInProcess);
    config.setBool(prefix + ".trackIntegrity", trackIntegrity);
    config.setBool(prefix + ".keepHistory", keepHistory);
  }

  public boolean isContainsInConfig(Config config)
  {
    return config.containsSection(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(id));
  }

  public static void removeTaskFromConfig(Config config, String taskId)
  {
    config.removeSection(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId));
  }

  public void removeFromConfig(Config config)
  {
    removeTaskFromConfig(config, id);
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof Task) {
      Task task = (Task) obj;
      return this.id.equals(task.id)
              && this.name.equals(task.name)
              && this.provider.equals(task.provider)
              && this.enabled == task.enabled
              && this.priority == task.priority
              && this.retryOnFail == task.retryOnFail
              && this.replaceMessage == task.replaceMessage
              && this.svcType.equals(task.svcType)
              && this.endDate.equals(task.endDate)
              && this.retryTime.equals(task.retryTime)
              && this.validityPeriod.equals(task.validityPeriod)
              && this.validityDate.equals(task.validityDate)
              && this.activePeriodStart.equals(task.activePeriodStart)
              && this.activePeriodEnd.equals(task.activePeriodEnd)
              && this.query.equals(task.query)
              && this.template.equals(task.template)
              && this.dsTimeout == task.dsTimeout
              && this.messagesCacheSize == task.messagesCacheSize
              && this.messagesCacheSleep == task.messagesCacheSleep
              && this.transactionMode == task.transactionMode
              && this.uncommitedInGeneration == task.uncommitedInGeneration
              && this.uncommitedInProcess == task.uncommitedInProcess
              && this.trackIntegrity == task.trackIntegrity
              && this.keepHistory == task.keepHistory;
    } else
      return false;
  }

  /************************************** properties **************************************/

  public String getId() {
    return id;
  }
  public void setId(String id) {
    this.id = id;
  }

  public String getName() {
    return name;
  }
  public void setName(String name) {
    this.name = name;
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

  public int getPriority() {
    return priority;
  }
  public void setPriority(int priority) {
    this.priority = priority;
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

  public String getRetryTime() {
    return retryTime;
  }
  public void setRetryTime(String retryTime) {
    this.retryTime = retryTime;
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

  public int getDsTimeout() {
    return dsTimeout;
  }
  public void setDsTimeout(int dsTimeout) {
    this.dsTimeout = dsTimeout;
  }

  public int getMessagesCacheSize() {
    return messagesCacheSize;
  }
  public void setMessagesCacheSize(int messagesCacheSize) {
    this.messagesCacheSize = messagesCacheSize;
  }
  public int getMessagesCacheSleep() {
    return messagesCacheSleep;
  }
  public void setMessagesCacheSleep(int messagesCacheSleep) {
    this.messagesCacheSleep = messagesCacheSleep;
  }

  public boolean isTransactionMode() {
    return transactionMode;
  }
  public void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
  }

  public int getUncommitedInGeneration() {
    return uncommitedInGeneration;
  }
  public void setUncommitedInGeneration(int uncommitedInGeneration) {
    this.uncommitedInGeneration = uncommitedInGeneration;
  }
  public int getUncommitedInProcess() {
    return uncommitedInProcess;
  }
  public void setUncommitedInProcess(int uncommitedInProcess) {
    this.uncommitedInProcess = uncommitedInProcess;
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
}
