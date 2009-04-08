package ru.novosoft.smsc.infosme.backend.config;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.Journal;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.config.driver.Driver;
import ru.novosoft.smsc.infosme.backend.config.driver.DriverManager;
import ru.novosoft.smsc.infosme.backend.config.provider.Provider;
import ru.novosoft.smsc.infosme.backend.config.provider.ProviderManager;
import ru.novosoft.smsc.infosme.backend.config.retrypolicies.RetryPolicy;
import ru.novosoft.smsc.infosme.backend.config.retrypolicies.RetryPolicyManager;
import ru.novosoft.smsc.infosme.backend.config.schedules.Schedule;
import ru.novosoft.smsc.infosme.backend.config.schedules.ScheduleManager;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.config.tasks.TaskManager;
import ru.novosoft.smsc.util.config.Config;

import java.util.List;
import java.util.Iterator;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class InfoSmeConfig {

  // Common options
  private String address;
  private String svcType;
  private int protocolId;
  private int maxMessagesPerSecond;
  private int unrespondedMessagesMax;
  private int unrespondedMessagesSleep;
  private String responceWaitTime;
  private String receiptWaitTime;
  private String tasksTaskTablesPrefix;
  private int tasksSwitchTimeout;

  private int tasksThreadPoolMax;
  private int tasksThreadPoolInit;

  private int eventsThreadPoolMax;
  private int eventsThreadPoolInit;

  private String adminHost;
  private int adminPort;

  private String smscHost;
  private int smscPort;
  private String smscSid;
  private int smscTimeout;
  private String smscPassword;

  private String storeLocation;
  private String statStoreLocation;

  private boolean optionsModified;

  private final InfoSmeContext ctx;
  private final String configDir;

  private final TaskManager taskManager;
  private final ScheduleManager scheduleManager;
  private final RetryPolicyManager retryPolicyManager;
  private final ProviderManager providerManager;
  private final DriverManager driverManager;


  public InfoSmeConfig(String configDir, InfoSmeContext ctx) throws AdminException {
    try {
      Config cfg = ctx.loadCurrentConfig();
      this.taskManager = new TaskManager(configDir, cfg);
      this.scheduleManager = new ScheduleManager(cfg);
      this.retryPolicyManager = new RetryPolicyManager(cfg);
      this.providerManager = new ProviderManager(cfg);
      this.driverManager = new DriverManager(cfg);
      this.ctx = ctx;
      this.configDir = configDir;
      loadOptions(cfg);
    } catch (Throwable e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  private static void checkConfiguration(TaskManager taskManager, ScheduleManager scheduleManager, RetryPolicyManager retryManager) throws AdminException {
    // Check schedules
    for (Iterator iter = scheduleManager.getSchedules().iterator(); iter.hasNext();) {
      Schedule s = (Schedule)iter.next();
      for (Iterator iter1 = s.getTasks().iterator(); iter1.hasNext();) {
        String taskId = (String)iter1.next();
        if (!taskManager.containsTaskWithId(taskId))
          throw new AdminException("Configuration error: schedule '" + s.getId() + "' contains task '" + taskId + "' which does not exists in configuration.");
      }
    }

    // Check tasks
    for (Iterator iter = taskManager.getTasks(null).iterator(); iter.hasNext();) {
      Task t = (Task)iter.next();
      if (t.isRetryOnFail() && !retryManager.containsRetryPolicy(t.getRetryPolicy()))
        throw new AdminException("Configuration error: task '" + t.getId() + "' contains retry policy '" + t.getRetryPolicy() + "' which does not exists in configuration.");
    }
  }

  public ConfigChanges apply(String user, String owner, boolean options, boolean tasks, boolean schedules, boolean retries, boolean providers, boolean drivers) throws AdminException {
    try {
      Config cfg = ctx.loadCurrentConfig();

      // Check saved configuration
      TaskManager tm = tasks ? taskManager : new TaskManager(configDir, cfg);
      ScheduleManager sm = schedules ? scheduleManager : new ScheduleManager(cfg);
      RetryPolicyManager rpm = retries ? retryPolicyManager : new RetryPolicyManager(cfg);
      checkConfiguration(tm, sm, rpm);

      // Apply changes
      Changes tasksChanges = null, schedulesChanges = null, policiesChanges = null;
      if (options)
        applyOptions(cfg);
      if (tasks)
        tasksChanges = taskManager.applyTasks(owner, cfg);
      if (schedules)
        schedulesChanges = scheduleManager.applySchedules(cfg);
      if (retries)
        policiesChanges = retryPolicyManager.applyRetryPolicies(cfg);
      if (providers)
        providerManager.applyProviders(cfg);
      if (drivers)
        driverManager.applyDrivers(cfg);

      cfg.save();

      // Journal changes
      Journal j = ctx.getAppContext().getJournal();
      if (tasks) {
        for (Iterator iter = tasksChanges.getAdded().iterator(); iter.hasNext();)
          j.append(user, "", SubjectTypes.TYPE_infosme, (String)iter.next(), Actions.ACTION_ADD, "Type", "Task");
        for (Iterator iter = tasksChanges.getDeleted().iterator(); iter.hasNext();)
          j.append(user, "", SubjectTypes.TYPE_infosme, (String)iter.next(), Actions.ACTION_DEL, "Type", "Task");
        for (Iterator iter = tasksChanges.getModified().iterator(); iter.hasNext();)
          j.append(user, "", SubjectTypes.TYPE_infosme, (String)iter.next(), Actions.ACTION_MODIFY, "Type", "Task");
      }

      if (schedules) {
        for (Iterator iter = schedulesChanges.getAdded().iterator(); iter.hasNext();)
          j.append(user, "", SubjectTypes.TYPE_infosme, (String)iter.next(), Actions.ACTION_ADD, "Type", "Schedule");
        for (Iterator iter = schedulesChanges.getDeleted().iterator(); iter.hasNext();)
          j.append(user, "", SubjectTypes.TYPE_infosme, (String)iter.next(), Actions.ACTION_DEL, "Type", "Schedule");
        for (Iterator iter = schedulesChanges.getModified().iterator(); iter.hasNext();)
          j.append(user, "", SubjectTypes.TYPE_infosme, (String)iter.next(), Actions.ACTION_MODIFY, "Type", "Schedule");
      }

      if (retries) {
        for (Iterator iter = policiesChanges.getAdded().iterator(); iter.hasNext();)
          j.append(user, "", SubjectTypes.TYPE_infosme, (String)iter.next(), Actions.ACTION_ADD, "Type", "Retry Policy");
        for (Iterator iter = policiesChanges.getDeleted().iterator(); iter.hasNext();)
          j.append(user, "", SubjectTypes.TYPE_infosme, (String)iter.next(), Actions.ACTION_DEL, "Type", "Retry Policy");
        for (Iterator iter = policiesChanges.getModified().iterator(); iter.hasNext();)
          j.append(user, "", SubjectTypes.TYPE_infosme, (String)iter.next(), Actions.ACTION_MODIFY, "Type", "Retry Policy");
      }

      if (options && optionsModified)
        j.append(user, "", SubjectTypes.TYPE_infosme, "Options", Actions.ACTION_SAVE, "Type", "Options");

      if (drivers && driverManager.isDriversChanged())
        j.append(user, "", SubjectTypes.TYPE_infosme, "Drivers", Actions.ACTION_MODIFY, "Type", "Drivers");

      if (providers && providerManager.isProvidersChanged())
        j.append(user, "", SubjectTypes.TYPE_infosme, "Providers", Actions.ACTION_MODIFY, "Type", "Providers");

      // Clear configuration
      if (options)
        optionsModified = false;
      if (tasks)
        taskManager.setModified(false, owner);
      if (schedules)
        scheduleManager.setModified(false);
      if (retries)
        retryPolicyManager.setModified(false);
      if (providers)
        providerManager.setModified(false);
      if (drivers)
        driverManager.setModified(false);

      return new ConfigChanges(schedulesChanges, tasksChanges, policiesChanges);
    } catch (Throwable e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void reset(String user, String owner, boolean options, boolean tasks, boolean schedules, boolean retries, boolean providers, boolean drivers) throws AdminException {
    try {
      Config cfg = ctx.loadCurrentConfig();            

      Journal j = ctx.getAppContext().getJournal();
      if (options) {
        resetOptions(cfg);
        j.append(user, "", SubjectTypes.TYPE_infosme, "Options", Actions.ACTION_RESTORE);
      }
      if (tasks) {
        taskManager.resetTasks(owner, cfg);
        j.append(user, "", SubjectTypes.TYPE_infosme, "Tasks", Actions.ACTION_RESTORE);
      }
      if (schedules) {
        scheduleManager.resetSchedules(cfg);
        j.append(user, "", SubjectTypes.TYPE_infosme, "Schedules", Actions.ACTION_RESTORE);
      }
      if (retries) {
        retryPolicyManager.resetRetryPolicies(cfg);
        j.append(user, "", SubjectTypes.TYPE_infosme, "Retry Policies", Actions.ACTION_RESTORE);
      }
      if (providers) {
        providerManager.resetProviders(cfg);
        j.append(user, "", SubjectTypes.TYPE_infosme, "Providers", Actions.ACTION_RESTORE);
      }
      if (drivers) {
        driverManager.resetDrivers(cfg);
        j.append(user, "", SubjectTypes.TYPE_infosme, "Drivers", Actions.ACTION_RESTORE);
      }

    } catch (Throwable e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }


  // TASKS ----------------------------------------

  public Task createTask() throws AdminException {
    return taskManager.createTask();
  }

  public void addTask(Task t) {
    taskManager.addTask(t);
  }

  public void addAndApplyTask(Task t) throws AdminException {
    try {
      Config cfg = ctx.loadCurrentConfig();
      boolean modified = taskManager.containsTaskWithId(t.getId());
      taskManager.addTask(t, cfg);
      cfg.save();
      ctx.getAppContext().getJournal().append(t.getOwner(), "", SubjectTypes.TYPE_infosme, t.getId(), modified ? Actions.ACTION_MODIFY : Actions.ACTION_ADD, "Type", "Task");
    } catch (Throwable e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void removeTask(String id) {
    taskManager.removeTask(id);
  }

  public void removeAndApplyTask(String user, String id) throws AdminException {
    try {
      Config cfg = ctx.loadCurrentConfig();
      if (taskManager.removeTask(id, cfg)) {
        cfg.save();
        ctx.getAppContext().getJournal().append(user, "", SubjectTypes.TYPE_infosme, id, Actions.ACTION_DEL, "Type", "Task");
      }
    } catch (Throwable e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public Task getTask(String id) {
    return taskManager.getTask(id);
  }

  public List getTasks(String owner) {
    return taskManager.getTasks(owner);
  }

  public boolean isTasksChanged(String owner) {
    return taskManager.isTasksChanged(owner);
  }

  public boolean containsTaskWithName(String name) {
    return taskManager.containsTaskWithName(name);
  }

  public boolean containsTaskWithId(String id) {
    return taskManager.containsTaskWithId(id);
  }

  // SCHEDULES ---------------------------------

  public void addSchedule(Schedule s) {
    scheduleManager.addSchedule(s);
  }

  public void removeSchedule(String id) {
    scheduleManager.removeSchedule(id);
  }

  public Schedule getSchedule(String id) {
    return scheduleManager.getSchedule(id);
  }

  public List getSchedules() {
    return scheduleManager.getSchedules();
  }

  public boolean containsSchedule(String id) {
    return scheduleManager.containsSchedule(id);
  }

  public boolean isSchedulesChanged() {
    return scheduleManager.isSchedulesChanged();
  }

  // RETRY POLICIES ----------------------------

  public void addRetryPolicy(RetryPolicy p) {
    retryPolicyManager.addRetryPolicy(p);
  }

  public void removeRetryPolicy(String id) {
    retryPolicyManager.removeRetryPolicy(id);
  }

  public RetryPolicy getRetryPolicy(String id) {
    return retryPolicyManager.getRetryPolicy(id);
  }

  public List getRetryPolicies() {
    return retryPolicyManager.getRetryPolicies();
  }

  public boolean containsRetryPolicy(String id) {
    return retryPolicyManager.containsRetryPolicy(id);
  }

  public boolean isRetryPoliciesChanged() {
    return retryPolicyManager.isRetryPoliciesChanged();
  }

  // PROVIDERS ------------------------------------

  public void addProvider(Provider p) {
    providerManager.addProvider(p);
  }

  public void removeProvider(String name) {
    providerManager.removeProvider(name);
  }

  public Provider getProvider(String name) {
    return providerManager.getProvider(name);
  }

  public List getProviders() {
    return providerManager.getProviders();
  }

  public boolean containsProvider(String name) {
    return providerManager.containsProvider(name);
  }

  public boolean isProvidersChanged() {
    return providerManager.isProvidersChanged();
  }

  // DRIVERS -----------------------------------

  public void addDriver(Driver d) {
    driverManager.addDriver(d);
  }

  public void removeDriver(String type) {
    driverManager.removeDriver(type);
  }

  public Driver getDriver(String type) {
    return driverManager.getDriver(type);
  }

  public List getDrivers() {
    return driverManager.getDrivers();
  }

  public void clearDrivers() {
    driverManager.clearDrivers();
  }

  public boolean isDriversChanged() {
    return driverManager.isDriversChanged();
  }

  // OPTIONS

  private void loadOptions(Config cfg) throws AdminException {
    try {
      address = cfg.getString("InfoSme.Address");
      svcType = cfg.getString("InfoSme.SvcType");
      protocolId = cfg.getInt("InfoSme.ProtocolId");
      maxMessagesPerSecond = cfg.getInt("InfoSme.maxMessagesPerSecond");
      unrespondedMessagesMax = cfg.getInt("InfoSme.unrespondedMessagesMax");
      unrespondedMessagesSleep = cfg.getInt("InfoSme.unrespondedMessagesSleep");
      responceWaitTime = cfg.getString("InfoSme.responceWaitTime");
      receiptWaitTime = cfg.getString("InfoSme.receiptWaitTime");

      tasksThreadPoolMax = cfg.getInt("InfoSme.TasksThreadPool.max");
      tasksThreadPoolInit = cfg.getInt("InfoSme.TasksThreadPool.init");

      eventsThreadPoolMax = cfg.getInt("InfoSme.EventsThreadPool.max");
      eventsThreadPoolInit = cfg.getInt("InfoSme.EventsThreadPool.init");

      adminHost = cfg.getString("InfoSme.Admin.host");
      adminPort = cfg.getInt("InfoSme.Admin.port");

      smscHost = cfg.getString("InfoSme.SMSC.host");
      smscPort = cfg.getInt("InfoSme.SMSC.port");
      smscSid = cfg.getString("InfoSme.SMSC.sid");
      smscTimeout = cfg.getInt("InfoSme.SMSC.timeout");
      smscPassword = cfg.getString("InfoSme.SMSC.password");
      tasksSwitchTimeout = cfg.getInt("InfoSme.tasksSwitchTimeout");
      tasksTaskTablesPrefix = cfg.getString("InfoSme.tasksTablesPrefix");

      storeLocation = cfg.getString("InfoSme.storeLocation");
      statStoreLocation = cfg.getString("InfoSme.statStoreLocation");

    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void applyOptions(Config cfg) throws AdminException {
    try {
      cfg.setString("InfoSme.Address", address);
      cfg.setString("InfoSme.SvcType", svcType);
      cfg.setInt("InfoSme.ProtocolId", protocolId);
      cfg.setInt("InfoSme.maxMessagesPerSecond", maxMessagesPerSecond);
      cfg.setInt("InfoSme.unrespondedMessagesMax", unrespondedMessagesMax);
      cfg.setInt("InfoSme.unrespondedMessagesSleep", unrespondedMessagesSleep);
      cfg.setString("InfoSme.responceWaitTime", responceWaitTime);
      cfg.setString("InfoSme.receiptWaitTime", receiptWaitTime);

      cfg.setInt("InfoSme.TasksThreadPool.max", tasksThreadPoolMax);
      cfg.setInt("InfoSme.TasksThreadPool.init", tasksThreadPoolInit);

      cfg.setInt("InfoSme.EventsThreadPool.max", eventsThreadPoolMax);
      cfg.setInt("InfoSme.EventsThreadPool.init", eventsThreadPoolInit);

      cfg.setString("InfoSme.Admin.host", adminHost);
      cfg.setInt("InfoSme.Admin.port", adminPort);

      cfg.setString("InfoSme.SMSC.host", smscHost);
      cfg.setInt("InfoSme.SMSC.port", smscPort);
      cfg.setString("InfoSme.SMSC.sid", smscSid);
      cfg.setInt("InfoSme.SMSC.timeout", smscTimeout);
      cfg.setString("InfoSme.SMSC.password", smscPassword);
      cfg.setInt("InfoSme.tasksSwitchTimeout", tasksSwitchTimeout);
      cfg.setString("InfoSme.tasksTablesPrefix", tasksTaskTablesPrefix);

      cfg.setString("InfoSme.storeLocation", storeLocation);
      cfg.setString("InfoSme.statStoreLocation", statStoreLocation);

    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void resetOptions(Config cfg) throws AdminException {
    loadOptions(cfg);
    optionsModified = false;
  }

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
    setOptionsModified(true);
  }

  public String getAdminHost() {
    return adminHost;
  }

  public void setAdminHost(String adminHost) {
    this.adminHost = adminHost;
    setOptionsModified(true);
  }

  public int getAdminPort() {
    return adminPort;
  }

  public void setAdminPort(int adminPort) {
    this.adminPort = adminPort;
    setOptionsModified(true);
  }

  public int getEventsThreadPoolInit() {
    return eventsThreadPoolInit;
  }

  public void setEventsThreadPoolInit(int eventsThreadPoolInit) {
    this.eventsThreadPoolInit = eventsThreadPoolInit;
    setOptionsModified(true);
  }

  public int getEventsThreadPoolMax() {
    return eventsThreadPoolMax;
  }

  public void setEventsThreadPoolMax(int eventsThreadPoolMax) {
    this.eventsThreadPoolMax = eventsThreadPoolMax;
    setOptionsModified(true);
  }

  public int getMaxMessagesPerSecond() {
    return maxMessagesPerSecond;
  }

  public void setMaxMessagesPerSecond(int maxMessagesPerSecond) {
    this.maxMessagesPerSecond = maxMessagesPerSecond;
    setOptionsModified(true);
  }

  public boolean isOptionsModified() {
    return optionsModified;
  }

  public void setOptionsModified(boolean optionsModified) {
    this.optionsModified = optionsModified;
  }

  public int getProtocolId() {
    return protocolId;
  }

  public void setProtocolId(int protocolId) {
    this.protocolId = protocolId;
    setOptionsModified(true);
  }

  public String getReceiptWaitTime() {
    return receiptWaitTime;
  }

  public void setReceiptWaitTime(String receiptWaitTime) {
    this.receiptWaitTime = receiptWaitTime;
    setOptionsModified(true);
  }

  public String getResponceWaitTime() {
    return responceWaitTime;
  }

  public void setResponceWaitTime(String responceWaitTime) {
    this.responceWaitTime = responceWaitTime;
    setOptionsModified(true);
  }

  public String getSmscHost() {
    return smscHost;
  }

  public void setSmscHost(String smscHost) {
    this.smscHost = smscHost;
    setOptionsModified(true);
  }

  public String getSmscPassword() {
    return smscPassword;
  }

  public void setSmscPassword(String smscPassword) {
    this.smscPassword = smscPassword;
    setOptionsModified(true);
  }

  public int getSmscPort() {
    return smscPort;
  }

  public void setSmscPort(int smscPort) {
    this.smscPort = smscPort;
    setOptionsModified(true);
  }

  public String getSmscSid() {
    return smscSid;
  }

  public void setSmscSid(String smscSid) {
    this.smscSid = smscSid;
    setOptionsModified(true);
  }

  public int getSmscTimeout() {
    return smscTimeout;
  }

  public void setSmscTimeout(int smscTimeout) {
    this.smscTimeout = smscTimeout;
    setOptionsModified(true);
  }

  public String getStatStoreLocation() {
    return statStoreLocation;
  }

  public void setStatStoreLocation(String statStoreLocation) {
    this.statStoreLocation = statStoreLocation;
    setOptionsModified(true);
  }

  public String getStoreLocation() {
    return storeLocation;
  }

  public void setStoreLocation(String storeLocation) {
    this.storeLocation = storeLocation;
    setOptionsModified(true);
  }

  public String getSvcType() {
    return svcType;
  }

  public void setSvcType(String svcType) {
    this.svcType = svcType;
    setOptionsModified(true);
  }

  public int getTasksSwitchTimeout() {
    return tasksSwitchTimeout;
  }

  public void setTasksSwitchTimeout(int tasksSwitchTimeout) {
    this.tasksSwitchTimeout = tasksSwitchTimeout;
    setOptionsModified(true);
  }

  public String getTasksTaskTablesPrefix() {
    return tasksTaskTablesPrefix;
  }

  public void setTasksTaskTablesPrefix(String tasksTaskTablesPrefix) {
    this.tasksTaskTablesPrefix = tasksTaskTablesPrefix;
    setOptionsModified(true);
  }

  public int getTasksThreadPoolInit() {
    return tasksThreadPoolInit;
  }

  public void setTasksThreadPoolInit(int tasksThreadPoolInit) {
    this.tasksThreadPoolInit = tasksThreadPoolInit;
    setOptionsModified(true);
  }

  public int getTasksThreadPoolMax() {
    return tasksThreadPoolMax;
  }

  public void setTasksThreadPoolMax(int tasksThreadPoolMax) {
    this.tasksThreadPoolMax = tasksThreadPoolMax;
    setOptionsModified(true);
  }

  public int getUnrespondedMessagesMax() {
    return unrespondedMessagesMax;
  }

  public void setUnrespondedMessagesMax(int unrespondedMessagesMax) {
    this.unrespondedMessagesMax = unrespondedMessagesMax;
    setOptionsModified(true);
  }

  public int getUnrespondedMessagesSleep() {
    return unrespondedMessagesSleep;
  }

  public void setUnrespondedMessagesSleep(int unrespondedMessagesSleep) {
    this.unrespondedMessagesSleep = unrespondedMessagesSleep;
    setOptionsModified(true);
  }
}
