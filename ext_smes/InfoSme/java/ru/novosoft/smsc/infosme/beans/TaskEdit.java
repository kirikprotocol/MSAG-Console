package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;

import java.util.List;
import java.util.Collection;
import java.security.Principal;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 2:31:58 PM
 */
public class TaskEdit extends InfoSmeBean
{
  private String mbDone = null;
  private String mbCancel = null;

  private boolean initialized = false;
  private boolean create = false;
  private String oldTask = null;

  private String sectionName = null;
  private String name = null;
  private String provider = null;
  private boolean enabled = false;
  private int priority = 0;
  private boolean retryOnFail = false;
  private boolean replaceMessage = false;
  private String svcType = null;
  private String endDate = null;
  private String retryTime = null;
  private String validityPeriod = null;
  private String validityDate = null;
  private String activePeriodStart = null;
  private String activePeriodEnd = null;
  private String query = null;
  private String template = null;
  private int dsOwnTimeout = 0;
  private int dsIntTimeout = 0;
  private int messagesCacheSize = 0;
  private int messagesCacheSleep = 0;
  private boolean transactionMode = false;
  private int uncommitedInGeneration = 0;
  private int uncommitedInProcess = 0;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      if (!create) {
        if (sectionName == null || sectionName.length() == 0)
          return error("Task not specified");
        oldTask = sectionName;

        try {
          final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(sectionName);
          name = getConfig().getString(prefix + ".name");
          provider = getConfig().getString(prefix + ".dsId");
          enabled = getConfig().getBool(prefix + ".enabled");
          priority = getConfig().getInt(prefix + ".priority");
          retryOnFail = getConfig().getBool(prefix + ".retryOnFail");
          replaceMessage = getConfig().getBool(prefix + ".replaceMessage");
          svcType = getConfig().getString(prefix + ".svcType");
          endDate = getConfig().getString(prefix + ".endDate");
          retryTime = getConfig().getString(prefix + ".retryTime");
          validityPeriod = getConfig().getString(prefix + ".validityPeriod");
          validityDate = getConfig().getString(prefix + ".validityDate");
          activePeriodStart = getConfig().getString(prefix + ".activePeriodStart");
          activePeriodEnd = getConfig().getString(prefix + ".activePeriodEnd");
          query = getConfig().getString(prefix + ".query");
          template = getConfig().getString(prefix + ".template");
          dsOwnTimeout = getConfig().getInt(prefix + ".dsOwnTimeout");
          dsIntTimeout = getConfig().getInt(prefix + ".dsIntTimeout");
          messagesCacheSize = getConfig().getInt(prefix + ".messagesCacheSize");
          messagesCacheSleep = getConfig().getInt(prefix + ".messagesCacheSleep");
          transactionMode = getConfig().getBool(prefix + ".transactionMode");
          uncommitedInGeneration = getConfig().getInt(prefix + ".uncommitedInGeneration");
          uncommitedInProcess = getConfig().getInt(prefix + ".uncommitedInProcess");
        } catch (Exception e) {
          logger.error(e);
          return error(e.getMessage());
        }
      }
    }
    if (sectionName == null) sectionName = "";
    if (name == null) name = null;
    if (provider == null) provider = "";
    if (svcType == null) svcType = "";
    if (endDate == null) endDate = "";
    if (retryTime == null) retryTime = "";
    if (validityPeriod == null) validityPeriod = "";
    if (validityDate == null) validityDate = "";
    if (activePeriodStart == null) activePeriodStart = "";
    if (activePeriodEnd == null) activePeriodEnd = "";
    if (query == null) query = "";
    if (template == null) template = "";
    if (oldTask == null) oldTask = "";

    return result;
  }

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbDone != null)
      return done();
    if (mbCancel != null)
      return RESULT_DONE;

    return result;
  }

  protected int done()
  {
    if (sectionName == null || sectionName.length() == 0)
      return error("Task section name not specified");
    final String prefix = TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(sectionName);
    if (!create) {
      if (!oldTask.equals(sectionName)) {
        if (getConfig().containsSection(prefix))
          return error("Task already exists", sectionName);
        getConfig().removeSection(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(oldTask));
      }
    }
    getConfig().setString(prefix + ".name", name);
    getConfig().setString(prefix + ".dsId", provider);
    getConfig().setBool(prefix + ".enabled", enabled);
    getConfig().setInt(prefix + ".priority", priority);
    getConfig().setBool(prefix + ".retryOnFail", retryOnFail);
    getConfig().setBool(prefix + ".replaceMessage", replaceMessage);
    getConfig().setString(prefix + ".svcType", svcType);
    getConfig().setString(prefix + ".endDate", endDate);
    getConfig().setString(prefix + ".retryTime", retryTime);
    getConfig().setString(prefix + ".validityPeriod", validityPeriod);
    getConfig().setString(prefix + ".validityDate", validityDate);
    getConfig().setString(prefix + ".activePeriodStart", activePeriodStart);
    getConfig().setString(prefix + ".activePeriodEnd", activePeriodEnd);
    getConfig().setString(prefix + ".query", query);
    getConfig().setString(prefix + ".template", template);
    getConfig().setInt(prefix + ".dsOwnTimeout", dsOwnTimeout);
    getConfig().setInt(prefix + ".dsIntTimeout", dsIntTimeout);
    getConfig().setInt(prefix + ".messagesCacheSize", messagesCacheSize);
    getConfig().setInt(prefix + ".messagesCacheSleep", messagesCacheSleep);
    getConfig().setBool(prefix + ".transactionMode", transactionMode);
    getConfig().setInt(prefix + ".uncommitedInGeneration", uncommitedInGeneration);
    getConfig().setInt(prefix + ".uncommitedInProcess", uncommitedInProcess);

    return RESULT_DONE;
  }

  public Collection getAllProviders()
  {
    return new SortedList(getConfig().getSectionChildShortSectionNames("InfoSme.DataProvider"));
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public boolean isCreate()
  {
    return create;
  }

  public void setCreate(boolean create)
  {
    this.create = create;
  }

  public String getOldTask()
  {
    return oldTask;
  }

  public void setOldTask(String oldTask)
  {
    this.oldTask = oldTask;
  }

  public String getSectionName()
  {
    return sectionName;
  }

  public void setSectionName(String sectionName)
  {
    this.sectionName = sectionName;
  }

  public String getProvider()
  {
    return provider;
  }

  public void setProvider(String provider)
  {
    this.provider = provider;
  }

  public boolean isEnabled()
  {
    return enabled;
  }

  public void setEnabled(boolean enabled)
  {
    this.enabled = enabled;
  }

  public int getPriorityInt()
  {
    return priority;
  }

  public void setPriorityInt(int priority)
  {
    this.priority = priority;
  }

  public String getPriority()
  {
    return String.valueOf(priority);
  }

  public void setPriority(String priority)
  {
    try {
      this.priority = Integer.decode(priority).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set priority to value \"" + priority + "\"", e);
      this.priority = 0;
    }
  }

  public boolean isRetryOnFail()
  {
    return retryOnFail;
  }

  public void setRetryOnFail(boolean retryOnFail)
  {
    this.retryOnFail = retryOnFail;
  }

  public boolean isReplaceMessage()
  {
    return replaceMessage;
  }

  public void setReplaceMessage(boolean replaceMessage)
  {
    this.replaceMessage = replaceMessage;
  }

  public String getSvcType()
  {
    return svcType;
  }

  public void setSvcType(String svcType)
  {
    this.svcType = svcType;
  }

  public String getEndDate()
  {
    return endDate;
  }

  public void setEndDate(String endDate)
  {
    this.endDate = endDate;
  }

  public String getRetryTime()
  {
    return retryTime;
  }

  public void setRetryTime(String retryTime)
  {
    this.retryTime = retryTime;
  }

  public String getValidityPeriod()
  {
    return validityPeriod;
  }

  public void setValidityPeriod(String validityPeriod)
  {
    this.validityPeriod = validityPeriod;
  }

  public String getValidityDate()
  {
    return validityDate;
  }

  public void setValidityDate(String validityDate)
  {
    this.validityDate = validityDate;
  }

  public String getActivePeriodStart()
  {
    return activePeriodStart;
  }

  public void setActivePeriodStart(String activePeriodStart)
  {
    this.activePeriodStart = activePeriodStart;
  }

  public String getActivePeriodEnd()
  {
    return activePeriodEnd;
  }

  public void setActivePeriodEnd(String activePeriodEnd)
  {
    this.activePeriodEnd = activePeriodEnd;
  }

  public String getQuery()
  {
    return query;
  }

  public void setQuery(String query)
  {
    this.query = query;
  }

  public String getTemplate()
  {
    return template;
  }

  public void setTemplate(String template)
  {
    this.template = template;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }

  public int getDsOwnTimeoutInt()
  {
    return dsOwnTimeout;
  }

  public void setDsOwnTimeoutInt(int dsOwnTimeout)
  {
    this.dsOwnTimeout = dsOwnTimeout;
  }

  public int getDsIntTimeoutInt()
  {
    return dsIntTimeout;
  }

  public void setDsIntTimeoutInt(int dsIntTimeout)
  {
    this.dsIntTimeout = dsIntTimeout;
  }

  public String getDsOwnTimeout()
  {
    return String.valueOf(dsOwnTimeout);
  }

  public void setDsOwnTimeout(String dsOwnTimeout)
  {
    try {
      this.dsOwnTimeout = Integer.decode(dsOwnTimeout).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set dsOwnTimeout to value \"" + dsOwnTimeout + "\"", e);
      this.dsOwnTimeout = 0;
    }
  }

  public String getDsIntTimeout()
  {
    return String.valueOf(dsIntTimeout);
  }

  public void setDsIntTimeout(String dsIntTimeout)
  {
    try {
      this.dsIntTimeout = Integer.decode(dsIntTimeout).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set dsIntTimeout to value \"" + dsIntTimeout + "\"", e);
      this.dsIntTimeout = 0;
    }
  }

  public int getMessagesCacheSizeInt()
  {
    return messagesCacheSize;
  }

  public void setMessagesCacheSizeInt(int messagesCacheSize)
  {
    this.messagesCacheSize = messagesCacheSize;
  }

  public int getMessagesCacheSleepInt()
  {
    return messagesCacheSleep;
  }

  public void setMessagesCacheSleepInt(int messagesCacheSleep)
  {
    this.messagesCacheSleep = messagesCacheSleep;
  }

  public boolean isTransactionMode()
  {
    return transactionMode;
  }

  public void setTransactionMode(boolean transactionMode)
  {
    this.transactionMode = transactionMode;
  }

  public int getUncommitedInGenerationInt()
  {
    return uncommitedInGeneration;
  }

  public void setUncommitedInGenerationInt(int uncommitedInGeneration)
  {
    this.uncommitedInGeneration = uncommitedInGeneration;
  }

  public int getUncommitedInProcessInt()
  {
    return uncommitedInProcess;
  }

  public void setUncommitedInProcessInt(int uncommitedInProcess)
  {
    this.uncommitedInProcess = uncommitedInProcess;
  }

  public String getMessagesCacheSize()
  {
    return String.valueOf(messagesCacheSize);
  }

  public void setMessagesCacheSize(String messagesCacheSize)
  {
    try {
      this.messagesCacheSize = Integer.decode(messagesCacheSize).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set messagesCacheSize to value \"" + messagesCacheSize + "\"", e);
      this.dsIntTimeout = 0;
    }
  }

  public String getMessagesCacheSleep()
  {
    return String.valueOf(messagesCacheSleep);
  }

  public void setMessagesCacheSleep(String messagesCacheSleep)
  {
    try {
      this.messagesCacheSleep = Integer.decode(messagesCacheSleep).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set messagesCacheSleep to value \"" + messagesCacheSleep + "\"", e);
      this.dsIntTimeout = 0;
    }
  }

  public String getUncommitedInGeneration()
  {
    return String.valueOf(uncommitedInGeneration);
  }

  public void setUncommitedInGeneration(String uncommitedInGeneration)
  {
    try {
      this.uncommitedInGeneration = Integer.decode(uncommitedInGeneration).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set uncommitedInGeneration to value \"" + uncommitedInGeneration + "\"", e);
      this.dsIntTimeout = 0;
    }
  }

  public String getUncommitedInProcess()
  {
    return String.valueOf(uncommitedInProcess);
  }

  public void setUncommitedInProcess(String uncommitedInProcess)
  {
    try {
      this.uncommitedInProcess = Integer.decode(uncommitedInProcess).intValue();
    } catch (Throwable e) {
      logger.error("Couldn't set uncommitedInGeneration to value \"" + uncommitedInGeneration + "\"", e);
      this.dsIntTimeout = 0;
    }
  }
}
