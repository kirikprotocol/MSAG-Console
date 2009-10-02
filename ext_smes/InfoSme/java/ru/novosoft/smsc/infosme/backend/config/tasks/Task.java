package ru.novosoft.smsc.infosme.backend.config.tasks;

import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;
import java.text.SimpleDateFormat;
import java.io.File;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 03.10.2003
 * Time: 17:10:35
 */
public class Task extends Observable
{
  public final static String[] WEEK_DAYS = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
  private final static String DEFAULT_ACTIVE_WEEK_DAYS = "Mon,Tue,Wed,Thu,Fri";
  private final static SimpleDateFormat df = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
  private static final SimpleDateFormat tf = new SimpleDateFormat("HH:mm:ss");

  public static final String INFOSME_EXT_PROVIDER = "EXTERNAL";

  private boolean modified;

  // General
  private final String id;
  private String owner = "";
  private String name = "";
  private String address = "";
  private String provider = "";
  private boolean enabled = false;
  private int priority = 0;
  private boolean replaceMessage = false;
  private String svcType = "";
  private boolean useDataSm = false;

  // Retry on fail
  private boolean retryOnFail = false;
  private String retryPolicy = "";
//  private String retryTime = "";


  // Time arguments
  private Date endDate = null;
  private Date startDate = null;
  private Date validityPeriod = null;
  private Date validityDate = null;
  private Date activePeriodStart = null;
  private Date activePeriodEnd = null;
  private Collection activeWeekDaysSet = new HashSet(WEEK_DAYS.length);

  // Other
  private String query = "";
  private String template = "";
  private String text = "";
  private int dsTimeout = 0;
  private int messagesCacheSize = 0;
  private int messagesCacheSleep = 0;
  private boolean transactionMode = false;
  private boolean keepHistory = false;
    // temporary switched to true
  private boolean saveFinalState = false;
  private boolean flash = false;
  private int uncommitedInGeneration = 0;
  private int uncommitedInProcess = 0;
  private boolean trackIntegrity;
  private boolean delivery = false;

  // for new deliveries
  private int regionId;
  private File deliveriesFile;
  private boolean deliveriesFileContainsTexts;
  private int actualRecordsSize;
  private boolean messagesHaveLoaded = false;
  // for secret
  private boolean secret;
  private boolean secretFlash;
  private String secretMessage="";

  Task(String id) {
    this.id = id;
    activeWeekDaysSet = new HashSet(WEEK_DAYS.length);
    Functions.addValuesToCollection(this.activeWeekDaysSet, DEFAULT_ACTIVE_WEEK_DAYS, ",", true);
    if (delivery)
      provider = Task.INFOSME_EXT_PROVIDER;
    this.modified = true;
  }

  Task(Config config, String id) throws Config.WrongParamTypeException, Config.ParamNotFoundException {
    this(id);
    final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(id);

    name = config.getString(prefix + ".name");
    address = (config.containsParameter(prefix + ".address")) ? config.getString(prefix + ".address") : "";
    owner = config.containsParameter(prefix + ".owner") ? config.getString(prefix + ".owner") : null;

    provider = config.getString(prefix + ".dsId");

    priority = config.getInt(prefix + ".priority");

    enabled = config.getBool(prefix + ".enabled");
    endDate = getDateFromConfig(config, prefix + ".endDate", df);
    startDate = getDateFromConfig(config, prefix + ".startDate", df);
    activePeriodStart = getDateFromConfig(config, prefix + ".activePeriodStart", tf);
    activePeriodEnd = getDateFromConfig(config, prefix + ".activePeriodEnd", tf);
    String activeWeekDaysStr = config.containsParameter(prefix + ".activeWeekDays") ? config.getString(prefix + ".activeWeekDays") : DEFAULT_ACTIVE_WEEK_DAYS;
    activeWeekDaysSet = new HashSet(WEEK_DAYS.length);
    Functions.addValuesToCollection(this.activeWeekDaysSet, activeWeekDaysStr, ",", true);

    transactionMode = config.getBool(prefix + ".transactionMode");
    validityPeriod = getDateFromConfig(config, prefix + ".validityPeriod", tf);
    validityDate = getDateFromConfig(config, prefix + ".validityDate", df);
    replaceMessage = config.getBool(prefix + ".replaceMessage");
    svcType = config.getString(prefix + ".svcType");
    retryOnFail = config.getBool(prefix + ".retryOnFail");
    retryPolicy = config.getString(prefix + ".retryPolicy");
    flash = config.containsParameter(prefix + ".flash") && config.getBool(prefix + ".flash");

    query = config.getString(prefix + ".query");
    template = config.getString(prefix + ".template");
    dsTimeout = config.getInt(prefix + ".dsTimeout");
    messagesCacheSize = config.getInt(prefix + ".messagesCacheSize");
    messagesCacheSleep = config.getInt(prefix + ".messagesCacheSleep");
    uncommitedInGeneration = config.getInt(prefix + ".uncommitedInGeneration");
    uncommitedInProcess = config.getInt(prefix + ".uncommitedInProcess");
    trackIntegrity = config.getBool(prefix + ".trackIntegrity");
    keepHistory = config.getBool(prefix + ".keepHistory");
    if (config.containsParameter(prefix + ".saveFinalState"))
      saveFinalState = config.getBool(prefix + ".saveFinalState");
    messagesHaveLoaded = config.getBool(prefix + ".messagesHaveLoaded");
    if (config.containsParameter(prefix + ".useDataSm"))
      useDataSm = config.getBool(prefix + ".useDataSm");

    delivery  = config.containsParameter(prefix + ".delivery") && config.getBool(prefix + ".delivery");
    if (delivery)
      provider = Task.INFOSME_EXT_PROVIDER;

    try {
      secret = config.getBool(prefix + ".secret");
      secretFlash = config.getBool(prefix + ".secretFlash");
      secretMessage = config.getString(prefix + ".secretMessage");
    } catch (Config.ParamNotFoundException e) {
      secret = false;
      secretFlash = false;
      secretMessage = "";
    }

    this.modified = false;
  }

  private static Date getDateFromConfig(Config config, String paramName, SimpleDateFormat df) {
    try {
      return df.parse(config.getString(paramName));
    } catch (Throwable e) {
      return null;
    }
  }

  void storeToConfig(Config config)
  {
    final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(id);
    config.setString(prefix + ".name", name);
    if (address != null && address.trim().length() > 0)
      config.setString(prefix + ".address", address.trim());
    config.setString(prefix + ".dsId", provider);
    config.setBool(prefix + ".enabled", enabled);
    config.setBool(prefix + ".delivery", delivery);
    config.setInt(prefix + ".priority", priority);
    config.setBool(prefix + ".retryOnFail", retryOnFail);
    config.setBool(prefix + ".replaceMessage", replaceMessage);
    config.setString(prefix + ".svcType", svcType);
    config.setString(prefix + ".startDate", startDate == null ? "" : df.format(startDate));
    config.setString(prefix + ".endDate", endDate == null ? "" : df.format(endDate));
    config.setString(prefix + ".retryPolicy", retryPolicy);
    config.setString(prefix + ".validityPeriod", validityPeriod == null ? "" : tf.format(validityPeriod));
    config.setString(prefix + ".validityDate", validityDate == null ? "" : df.format(validityDate));
    config.setString(prefix + ".activePeriodStart", activePeriodStart == null ? "" : tf.format(activePeriodStart));
    config.setString(prefix + ".activePeriodEnd", activePeriodEnd == null ? "" : tf.format(activePeriodEnd));
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
    config.setBool(prefix + ".saveFinalState", saveFinalState );
    config.setBool(prefix + ".useDataSm", useDataSm );
    config.setBool(prefix + ".flash", flash);
    config.setString(prefix + ".activeWeekDays", Functions.collectionToString(activeWeekDaysSet, ","));
    config.setBool(prefix + ".messagesHaveLoaded", messagesHaveLoaded);
    config.setBool(prefix + ".secret", secret);
    config.setBool(prefix + ".secretFlash", secretFlash);
    config.setString(prefix + ".secretMessage", secretMessage);
    config.setString(prefix + ".owner", owner);
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof Task) {
      Task task = (Task) obj;
      return this.id.equals(task.id)
              && this.name.equals(task.name)
              && this.address.equals(task.address)
              && this.provider.equals(task.provider)
              && this.enabled == task.enabled
              && this.priority == task.priority
              && this.retryOnFail == task.retryOnFail
              && this.replaceMessage == task.replaceMessage
              && this.svcType.equals(task.svcType)
              && this.endDate.equals(task.endDate)
              && this.startDate.equals(task.startDate)
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
              && this.keepHistory == task.keepHistory
              && this.saveFinalState == task.saveFinalState
              && this.useDataSm == task.useDataSm
              && this.activeWeekDaysSet.equals(task.activeWeekDaysSet)
              && this.retryPolicy.equals(task.retryPolicy)
              && this.secret == task.secret
              && this.secretFlash == task.secretFlash
              && this.secretMessage.equals(task.secretMessage)
              && this.owner.equals(task.owner);
    } else
      return false;
  }

  public String toString() {
    final StringBuffer sb = new StringBuffer();
    sb.append("Task");
    sb.append("{activePeriodEnd='").append(activePeriodEnd).append('\'');
    sb.append(", modified=").append(modified);
    sb.append(", id='").append(id).append('\'');
    sb.append(", owner='").append(owner).append('\'');
    sb.append(", name='").append(name).append('\'');
    sb.append(", address='").append(address).append('\'');
    sb.append(", provider='").append(provider).append('\'');
    sb.append(", enabled=").append(enabled);
    sb.append(", priority=").append(priority);
    sb.append(", retryOnFail=").append(retryOnFail);
    sb.append(", replaceMessage=").append(replaceMessage);
    sb.append(", svcType='").append(svcType).append('\'');
    sb.append(", endDate='").append(endDate).append('\'');
    sb.append(", startDate='").append(startDate).append('\'');
    sb.append(", validityPeriod='").append(validityPeriod).append('\'');
    sb.append(", validityDate='").append(validityDate).append('\'');
    sb.append(", activePeriodStart='").append(activePeriodStart).append('\'');
    sb.append(", activeWeekDays=").append(activeWeekDaysSet);
    sb.append(", activeWeekDaysSet=").append(activeWeekDaysSet);
    sb.append(", query='").append(query).append('\'');
    sb.append(", template='").append(template).append('\'');
    sb.append(", text='").append(text).append('\'');
    sb.append(", dsTimeout=").append(dsTimeout);
    sb.append(", retryPolicy='").append(retryPolicy).append('\'');
    sb.append(", messagesCacheSize=").append(messagesCacheSize);
    sb.append(", messagesCacheSleep=").append(messagesCacheSleep);
    sb.append(", transactionMode=").append(transactionMode);
    sb.append(", keepHistory=").append(keepHistory);
    sb.append(", saveFinalState=").append(saveFinalState);
    sb.append(", useDataSm=").append(useDataSm);
    sb.append(", flash=").append(flash);
    sb.append(", uncommitedInGeneration=").append(uncommitedInGeneration);
    sb.append(", uncommitedInProcess=").append(uncommitedInProcess);
    sb.append(", trackIntegrity=").append(trackIntegrity);
    sb.append(", delivery=").append(delivery);
    sb.append(", regionId='").append(regionId).append('\'');
    sb.append(", actualRecordsSize=").append(actualRecordsSize);
    sb.append(", messagesHaveLoaded=").append(messagesHaveLoaded);
    sb.append(", secret=").append(secret);
    sb.append(", secretFlash=").append(secretFlash);
    sb.append(", secretMessage='").append(secretMessage).append('\'');
    sb.append('}');
    return sb.toString();
  }

  /* ************************************* properties ************************************* */

  public String getId() {
    return id;
  }

  public String getName() {
    return name;
  }
  public void setName(String name) {
    this.name = name;
    modified = true;
  }

  public String getAddress() {
    return address;
  }
  public void setAddress(String address) {
    this.address = address;
    modified = true;
  }

  public String getProvider() {
    return provider;
  }

  public void setProvider(String provider) {
    this.provider = provider;
    modified = true;
  }

  public boolean isEnabled() {
    return enabled;
  }

  public void setEnabled(boolean enabled) {
    this.enabled = enabled;
    modified = true;
  }

  public boolean isDelivery() {
    return delivery;
  }

  public void setDelivery(boolean delivery) {
    this.delivery = delivery;
    modified = true;
  }

  public int getPriority() {
    return priority;
  }

  public void setPriority(int priority) {
    this.priority = priority;
    modified = true;
  }

  public boolean isRetryOnFail() {
    return retryOnFail;
  }

  public void setRetryOnFail(boolean retryOnFail) {
    this.retryOnFail = retryOnFail;
    modified = true;
  }

  public boolean isReplaceMessage() {
    return replaceMessage;
  }

  public void setReplaceMessage(boolean replaceMessage) {
    this.replaceMessage = replaceMessage;
    modified = true;
  }

  public String getSvcType() {
    return svcType;
  }

  public void setSvcType(String svcType) {
    this.svcType = svcType;
    modified = true;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) {
    this.endDate = endDate;
    modified = true;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) {
    this.startDate = startDate;
    modified = true;
  }

  public Date getValidityPeriod() {
    return validityPeriod;
  }

  public void setValidityPeriod(Date validityPeriod) {
    this.validityPeriod = validityPeriod;
    modified = true;
  }

  public Date getValidityDate() {
    return validityDate;
  }

  public void setValidityDate(Date validityDate) {
    this.validityDate = validityDate;
    modified = true;
  }

  public Date getActivePeriodStart() {
    return activePeriodStart;
  }

  public void setActivePeriodStart(Date activePeriodStart) {
    this.activePeriodStart = activePeriodStart;
    modified = true;
  }

  public Date getActivePeriodEnd() {
    return activePeriodEnd;
  }

  public void setActivePeriodEnd(Date activePeriodEnd) {
    this.activePeriodEnd = activePeriodEnd;
    modified = true;
  }

  public String getQuery() {
    return query;
  }

  public void setQuery(String query) {
    this.query = query;
    modified = true;
  }

  public String getTemplate() {
    return template;
  }

  public void setTemplate(String template) {
    this.template = template;
    modified = true;
  }

  public String getText() {
    return text;
  }
  public void setText(String text) {
    this.text = text;
    modified = true;
  }

  public int getDsTimeout() {
    return dsTimeout;
  }
  public void setDsTimeout(int dsTimeout) {
    this.dsTimeout = dsTimeout;
    modified = true;
  }

  public int getMessagesCacheSize() {
    return messagesCacheSize;
  }

  public void setMessagesCacheSize(int messagesCacheSize) {
    this.messagesCacheSize = messagesCacheSize;
    modified = true;
  }

  public int getMessagesCacheSleep() {
    return messagesCacheSleep;
  }

  public void setMessagesCacheSleep(int messagesCacheSleep) {
    this.messagesCacheSleep = messagesCacheSleep;
    modified = true;
  }

  public boolean isTransactionMode() {
    return transactionMode;
  }

  public void setTransactionMode(boolean transactionMode) {
    this.transactionMode = transactionMode;
    modified = true;
  }

  public int getUncommitedInGeneration() {
    return uncommitedInGeneration;
  }

  public void setUncommitedInGeneration(int uncommitedInGeneration) {
    this.uncommitedInGeneration = uncommitedInGeneration;
    modified = true;
  }

  public int getUncommitedInProcess() {
    return uncommitedInProcess;
  }

  public void setUncommitedInProcess(int uncommitedInProcess) {
    this.uncommitedInProcess = uncommitedInProcess;
    modified = true;
  }

  public boolean isTrackIntegrity() {
    return trackIntegrity;
  }

  public void setTrackIntegrity(boolean trackIntegrity) {
    this.trackIntegrity = trackIntegrity;
    modified = true;
  }

  public boolean isKeepHistory() {
    return keepHistory;
  }

  public void setKeepHistory(boolean keepHistory) {
    this.keepHistory = keepHistory;
    modified = true;
  }

  public boolean isSaveFinalState() {
    return saveFinalState;
  }

  public void setSaveFinalState(boolean saveFinalState) {
    this.saveFinalState = saveFinalState;
    modified = true;
  }

  public boolean isUseDataSm() {
    return useDataSm;
  }

  public void setUseDataSm(boolean useDataSm) {
    this.useDataSm = useDataSm;
    modified = true;
  }

  public Collection getActiveWeekDays() {
    return activeWeekDaysSet;
  }

  public void setActiveWeekDays(Collection activeWeekDays) {
    this.activeWeekDaysSet = new HashSet(activeWeekDays);
    modified = true;
  }

  public boolean isWeekDayActive(String weekday) {
    return activeWeekDaysSet.contains(weekday);
  }

  public int getRegionId() {
    return regionId;
  }

  public void setRegionId(int id) {
    this.regionId = id;
    modified = true;
  }

  public int getActualRecordsSize() {
    return actualRecordsSize;
  }

  public void setActualRecordsSize(int actualRecordsSize) {
    this.actualRecordsSize = actualRecordsSize;
    modified = true;
  }

  public Collection getActiveWeekDaysSet() {
    return activeWeekDaysSet;
  }

  public void setActiveWeekDaysSet(Collection activeWeekDaysSet) {
    this.activeWeekDaysSet = activeWeekDaysSet;
    modified = true;
  }

  public boolean isFlash() {
    return flash;
  }

  public void setFlash(boolean flash) {
    this.flash = flash;
    modified = true;
  }

  public boolean isMessagesHaveLoaded() {
    return messagesHaveLoaded;
  }

  public void setMessagesHaveLoaded(boolean messagesHaveLoaded) {
    this.messagesHaveLoaded = messagesHaveLoaded;
    modified = true;
  }

  public String getRetryPolicy() {
    return retryPolicy;
  }

  public void setRetryPolicy(String retryPolicy) {
    this.retryPolicy = retryPolicy;
    modified = true;
  }

  public boolean isSecret() {
    return secret;
  }

  public void setSecret(boolean secret) {
    this.secret = secret;
    modified = true;
  }

  public boolean isSecretFlash() {
    return secretFlash;
  }

  public void setSecretFlash(boolean secretFlash) {
    this.secretFlash = secretFlash;
    modified = true;
  }

  public String getSecretMessage() {
    return secretMessage;
  }

  public void setSecretMessage(String secretMessage) {
    this.secretMessage = secretMessage;
    modified = true;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner == null ? "" : owner;
    modified = true;
  }

  public File getDeliveriesFile() {
    return deliveriesFile;
  }

  public void setDeliveriesFile(File deliveriesFile) {
    this.deliveriesFile = deliveriesFile;
  }

  public boolean isDeliveriesFileContainsTexts() {
    return deliveriesFileContainsTexts;
  }

  public void setDeliveriesFileContainsTexts(boolean deliveriesFileContainsTexts) {
    this.deliveriesFileContainsTexts = deliveriesFileContainsTexts;
  }

  public void setModified(boolean modified) {
    this.modified = modified;
  }

  public boolean isModified() {
    return modified;
  }
}
