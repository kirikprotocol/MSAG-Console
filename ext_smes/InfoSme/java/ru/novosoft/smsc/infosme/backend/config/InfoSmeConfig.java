package ru.novosoft.smsc.infosme.backend.config;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.admin.users.User;
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
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.*;
import java.text.SimpleDateFormat;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class InfoSmeConfig {


  public final static String[] WEEK_DAYS = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
  private final static String DEFAULT_ACTIVE_WEEK_DAYS = "Mon,Tue,Wed,Thu,Fri";
  private static final SimpleDateFormat tf = new SimpleDateFormat("HH:mm:ss");

  // Common options
  private Boolean ussdPushFeature;
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

  private Map smscConns = new HashMap();

  private SmscConnector defSmscConn;

  private String storeLocation;
  private String archiveLocation;
  private String statStoreLocation;

  private boolean optionsModified;

  private final InfoSmeContext ctx;
  private final String configDir;

  private final TaskManager taskManager;
  private final ScheduleManager scheduleManager;
  private final RetryPolicyManager retryPolicyManager;
  private final ProviderManager providerManager;
  private final DriverManager driverManager;


  private String siebelJDBCSource="";
  private String siebelJDBCDriver="";
  private String siebelJDBCUser="";
  private String siebelJDBCPass="";

  private int siebelTMPeriod = 20;
  private boolean siebelTMRemove;

  // General
  private boolean siebelTReplaceMessage = false;
  private String siebelTSvcType = "";

  // Retry on fail
  private boolean siebelTRetryOnFail = false;
  private String siebelTRetryPolicy = "";


  // Time arguments
  private Date siebelTPeriodStart = null;
  private Date siebelTPeriodEnd = null;
  private Collection siebelTWeekDaysSet = new HashSet(WEEK_DAYS.length);

  // Other
  private int siebelTCacheSize = 0;
  private int siebelTCacheSleep = 0;
  private boolean siebelTTrMode = false;
  private boolean siebelTKeepHistory = false;
  // temporary switched to true
  private int siebelTUncommitGeneration = 0;
  private int siebelTUncommitProcess = 0;
  private boolean siebelTTrackIntegrity;


  public InfoSmeConfig(String configDir, InfoSmeContext ctx) throws AdminException {
    try {
      Config cfg = ctx.loadCurrentConfig();
      this.scheduleManager = new ScheduleManager(cfg);
      this.retryPolicyManager = new RetryPolicyManager(cfg);
      this.providerManager = new ProviderManager(cfg);
      this.driverManager = new DriverManager(cfg);
      this.ctx = ctx;
      this.configDir = configDir;
      Functions.addValuesToCollection(this.siebelTWeekDaysSet, DEFAULT_ACTIVE_WEEK_DAYS, ",", true);
      loadOptions(cfg);
      this.taskManager = new TaskManager(configDir, cfg);
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

  public synchronized ConfigChanges apply(String user, String owner, boolean options, boolean tasks, boolean schedules, boolean retries, boolean providers, boolean drivers) throws AdminException {
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

  public synchronized void reset(String user, String owner, boolean options, boolean tasks, boolean schedules, boolean retries, boolean providers, boolean drivers) throws AdminException {
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

  public Task createTask(User owner) throws AdminException {
    Task t = createTask();
    resetTask(t, owner);
    return t;
  }
  public Task createTask(User owner, String name) throws AdminException {
    Task t = createTask(owner);
    t.setName(name);
    return t;
  }

  public void resetTask(Task task, User owner) throws AdminException {
    task.setProvider(Task.INFOSME_EXT_PROVIDER);
    task.setOwner(owner.getName());
    task.setDelivery(true);
    task.setPriority(owner.getPrefs().getInfosmePriority());
    task.setMessagesCacheSize(owner.getPrefs().getInfosmeCacheSize());
    task.setMessagesCacheSleep(owner.getPrefs().getInfosmeCacheSleep());
    task.setUncommitedInGeneration(owner.getPrefs().getInfosmeUncommitGeneration());
    task.setUncommitedInProcess(owner.getPrefs().getInfosmeUncommitProcess());
    task.setTrackIntegrity(owner.getPrefs().isInfosmeTrackIntegrity());
    task.setKeepHistory(owner.getPrefs().isInfosmeKeepHistory());
    task.setReplaceMessage(owner.getPrefs().isInfosmeReplaceMessage());
    task.setSvcType(owner.getPrefs().getInfosmeSvcType());

    Region r = ctx.getAppContext().getRegionsManager().getRegionById(task.getRegionId());
    task.setActivePeriodStart(r == null ? owner.getPrefs().getInfosmePeriodStart() : r.getLocalTime(owner.getPrefs().getInfosmePeriodStart()));
    task.setActivePeriodEnd(r == null ? owner.getPrefs().getInfosmePeriodEnd() : r.getLocalTime(owner.getPrefs().getInfosmePeriodEnd()));
    task.setActiveWeekDaysSet(owner.getPrefs().getInfosmeWeekDaysSet());
    task.setTransactionMode(owner.getPrefs().isInfosmeTrMode());
    task.setValidityPeriod(owner.getPrefs().getInfosmeValidityPeriod());
    String sa;
    if(owner.getPrefs().getInfosmeSourceAddress() != null && (sa = owner.getPrefs().getInfosmeSourceAddress().trim()).length() != 0) {
      task.setAddress(sa);
    }else {
      task.setAddress(ctx.getInfoSmeConfig().getAddress());
    }
    Boolean ussdPushFeature = ctx.getInfoSmeConfig().getUssdPushFeature();
    if(ussdPushFeature != null) {
      task.setUseUssdPush(
          owner.getPrefs().isInfosmeUssdPush() != null && owner.getPrefs().isInfosmeUssdPush().booleanValue() ? 1 : 0
      );
    }else {
      task.setUseUssdPush(-1);
    }

    task.setEnabled(true);

    task.setStartDate(new Date());
  }

  private static void validateTask(Task t) throws AdminException{
    if (t.getPriority() <= 0 || t.getPriority() > 1000) {
      throw new AdminException("Task priority should be positive and less than 1000: "+t.getPriority());
    }
    if(t.getAddress() != null && t.getAddress().length() > 0) {
      char c = t.getAddress().charAt(0);
      if(c != '+' && !Character.isDigit(c)) {
        throw new AdminException("Task address is wrong "+t.getAddress());
      }
      for(int i = 1; i <= t.getAddress().length() - 1; i++) {
        if(!Character.isDigit(t.getAddress().charAt(i))) {
          throw new AdminException("Task address is wrong "+t.getAddress());
        }
      }
    }
    if (t.isRetryOnFail() && (t.getRetryPolicy() == null || t.getRetryPolicy().length() == 0)) {
      throw new AdminException("Task retry time specified incorrectly");
    }
    if(t.getValidityDate() == null && t.getValidityPeriod() == null) {
      throw new AdminException("Task's validity period/date specified incorrectly");
    }
    if((t.getActivePeriodStart() == null && t.getActivePeriodEnd() != null) ||
        (t.getActivePeriodStart() != null && t.getActivePeriodEnd() == null)) {
      throw new AdminException("Task's active period specified incorrectly");
    }
  }

  public void addTask(Task t) throws AdminException{
    validateTask(t);
    taskManager.addTask(t);
  }

  public synchronized void addAndApplyTask(Task t) throws AdminException {
    validateTask(t);
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

  public synchronized void setSiebelTMStarted(boolean started) throws AdminException{
    try {
      Config cfg = ctx.loadCurrentConfig();
      cfg.setBool("InfoSme.Siebel.TaskManager.started", started);
      cfg.save();
    } catch (Throwable e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public boolean isSiebelTMStarted() throws AdminException{
    try {
      Config cfg = ctx.loadCurrentConfig();
      return cfg.getBool("InfoSme.Siebel.TaskManager.started");
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

  public Task getTaskByName(String name) throws AdminException {
    return taskManager.getTaskByName(name);
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

  public boolean containsTaskWithName(String name, String owner) {
    return taskManager.containsTaskWithName(name, owner);
  }

  public boolean containsTaskWithId(String id) {
    return taskManager.containsTaskWithId(id);
  }

  public boolean hasUssdPushFeature() {
    return taskManager.hasUssdPushFeature();
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
      if(cfg.containsParameter("InfoSme.ussdPushFeature")) {
        ussdPushFeature = Boolean.valueOf(cfg.getBool("InfoSme.ussdPushFeature"));
      }
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

      if(cfg.containsSection("InfoSme.Siebel")) {
        String prefix = "InfoSme.Siebel.DataProvider";
        siebelJDBCSource = cfg.getString(prefix + ".jdbc.source");
        siebelJDBCDriver = cfg.getString(prefix + ".jdbc.driver");
        siebelJDBCUser = cfg.getString(prefix + ".jdbc.user");
        siebelJDBCPass = cfg.getString(prefix + ".jdbc.pass");

        prefix = "InfoSme.Siebel.TaskManager";
        siebelTMPeriod = cfg.getInt(prefix + ".period");
        siebelTMRemove = cfg.getBool(prefix + ".remove");

        prefix = "InfoSme.Siebel.Task";
        siebelTRetryOnFail = cfg.getBool(prefix + ".retryOnFail");
        siebelTReplaceMessage = cfg.getBool(prefix + ".replaceMessage");
        siebelTSvcType = cfg.getString(prefix + ".svcType");
        siebelTRetryPolicy = cfg.getString(prefix + ".retryPolicy");
        String tmp = cfg.getString(prefix + ".activePeriodStart");
        siebelTPeriodStart = (tmp == null || tmp.trim().length() == 0) ? null : tf.parse(tmp);
        tmp = cfg.getString(prefix + ".activePeriodEnd");
        siebelTPeriodEnd = (tmp == null || tmp.trim().length() == 0) ? null : tf.parse(tmp);

        siebelTCacheSize = cfg.getInt(prefix + ".messagesCacheSize");
        siebelTCacheSleep = cfg.getInt(prefix + ".messagesCacheSleep");
        siebelTTrMode = cfg.getBool(prefix + ".transactionMode");
        siebelTUncommitGeneration = cfg.getInt(prefix + ".uncommitedInGeneration");
        siebelTUncommitProcess = cfg.getInt(prefix + ".uncommitedInProcess");
        siebelTTrackIntegrity = cfg.getBool(prefix + ".trackIntegrity");
        siebelTKeepHistory = cfg.getBool(prefix + ".keepHistory");
        siebelTWeekDaysSet = new HashSet(WEEK_DAYS.length);
        Functions.addValuesToCollection(this.siebelTWeekDaysSet, cfg.getString(prefix + ".activeWeekDays"), ",", true);
      }

      if(cfg.containsSection("InfoSme.SMSCConnectors")) {
        String defname = cfg.getString("InfoSme.SMSCConnectors.default");
        Set childs = cfg.getSectionChildShortSectionNames("InfoSme.SMSCConnectors");
        Iterator i = childs.iterator();

        Map newSmscs = new HashMap();
        while(i.hasNext()) {
          SmscConnector smsc = new SmscConnector();
          String smscName = (String)i.next();
          smsc.setName(smscName);
          smsc.setHost(cfg.getString("InfoSme.SMSCConnectors."+smscName+".host"));
          smsc.setPort(cfg.getInt("InfoSme.SMSCConnectors."+smscName+".port"));
          smsc.setSid(cfg.getString("InfoSme.SMSCConnectors."+smscName+".sid"));
          smsc.setTimeout(cfg.getInt("InfoSme.SMSCConnectors."+smscName+".timeout"));
          smsc.setPassword(cfg.getString("InfoSme.SMSCConnectors."+smscName+".password"));
          if (cfg.containsParameter("InfoSme.SMSCConnectors."+smscName+".systemType"))
            smsc.setSystemType(cfg.getString("InfoSme.SMSCConnectors."+smscName+".systemType"));
          if (cfg.containsParameter("InfoSme.SMSCConnectors."+smscName+".rangeOfAddress"))
            smsc.setRangeOfAddress(cfg.getString("InfoSme.SMSCConnectors."+smscName+".rangeOfAddress"));
          if (cfg.containsParameter("InfoSme.SMSCConnectors."+smscName+".interfaceVersion"))
            smsc.setInterfaceVersion(cfg.getInt("InfoSme.SMSCConnectors."+smscName+".interfaceVersion"));
          else
            smsc.setInterfaceVersion(-1);
          newSmscs.put(smscName, smsc);
        }

        if((defname != null)) {
          SmscConnector def = (SmscConnector)newSmscs.get(defname);
          if(def == null) {
            throw new AdminException("SmscConnector not found with name="+defname);
          }
          defSmscConn = def;
        } else if(!newSmscs.isEmpty()) {
          throw new AdminException("Default smsc not found");
        }

        smscConns = newSmscs;
      }

      tasksSwitchTimeout = cfg.getInt("InfoSme.tasksSwitchTimeout");
      tasksTaskTablesPrefix = cfg.getString("InfoSme.tasksTablesPrefix");

      storeLocation = cfg.getString("InfoSme.storeLocation");
      archiveLocation = cfg.getString("InfoSme.archiveLocation");
      statStoreLocation = cfg.getString("InfoSme.statStoreLocation");

    } catch (Exception e) {
      e.printStackTrace();
      throw new AdminException(e.getMessage());
    }
  }

  public void applyOptions(Config cfg) throws AdminException {
    try {
      //Siebel
      String prefix = "InfoSme.Siebel.DataProvider";
      cfg.setString(prefix + ".jdbc.source", siebelJDBCSource);
      cfg.setString(prefix + ".jdbc.driver", siebelJDBCDriver);
      cfg.setString(prefix + ".jdbc.user", siebelJDBCUser);
      cfg.setString(prefix + ".jdbc.pass", siebelJDBCPass);

      prefix = "InfoSme.Siebel.TaskManager";
      cfg.setInt(prefix + ".period", siebelTMPeriod);
      cfg.setBool(prefix + ".remove", siebelTMRemove);

      prefix = "InfoSme.Siebel.Task";
      cfg.setBool(prefix + ".retryOnFail", siebelTRetryOnFail);
      cfg.setBool(prefix + ".replaceMessage", siebelTReplaceMessage);
      cfg.setString(prefix + ".svcType", siebelTSvcType);
      cfg.setString(prefix + ".retryPolicy", siebelTRetryPolicy);
      cfg.setString(prefix + ".activePeriodStart", siebelTPeriodStart == null ? "" : tf.format(siebelTPeriodStart));
      cfg.setString(prefix + ".activePeriodEnd", siebelTPeriodEnd == null ? "" : tf.format(siebelTPeriodEnd));

      cfg.setInt(prefix + ".messagesCacheSize", siebelTCacheSize);
      cfg.setInt(prefix + ".messagesCacheSleep", siebelTCacheSleep);
      cfg.setBool(prefix + ".transactionMode", siebelTTrMode);
      cfg.setInt(prefix + ".uncommitedInGeneration", siebelTUncommitGeneration);
      cfg.setInt(prefix + ".uncommitedInProcess", siebelTUncommitProcess);
      cfg.setBool(prefix + ".trackIntegrity", siebelTTrackIntegrity);
      cfg.setBool(prefix + ".keepHistory", siebelTKeepHistory);
      cfg.setString(prefix + ".activeWeekDays", Functions.collectionToString(siebelTWeekDaysSet, ","));

      //
      cfg.setString("InfoSme.Address", address);
      if(ussdPushFeature != null) {
        cfg.setBool("InfoSme.ussdPushFeature", ussdPushFeature.booleanValue());
      }
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

      if(defSmscConn != null) {
        if(smscConns.get(defSmscConn.getName()) == null) {
          throw new AdminException("Default smsc not found with name="+ defSmscConn.getName());
        }
        cfg.setString("InfoSme.SMSCConnectors.default", defSmscConn.getName());
      } else if(!smscConns.isEmpty()) {
        throw new AdminException("Default smsc not found");
      }

      Iterator i = smscConns.values().iterator();
      while(i.hasNext()){
        SmscConnector smsc = (SmscConnector)i.next();
        cfg.setString("InfoSme.SMSCConnectors."+smsc.getName()+".host",smsc.getHost());
        cfg.setInt("InfoSme.SMSCConnectors."+smsc.getName()+".port",smsc.getPort());
        cfg.setInt("InfoSme.SMSCConnectors."+smsc.getName()+".timeout",smsc.getTimeout());
        cfg.setString("InfoSme.SMSCConnectors."+smsc.getName()+".sid",smsc.getSid());
        cfg.setString("InfoSme.SMSCConnectors."+smsc.getName()+".password",smsc.getPassword());
        if (smsc.getSystemType() != null)
          cfg.setString("InfoSme.SMSCConnectors."+smsc.getName()+".systemType",smsc.getSystemType());
        else
          cfg.removeParam("InfoSme.SMSCConnectors."+smsc.getName()+".systemType");
        if (smsc.getRangeOfAddress() != null)
          cfg.setString("InfoSme.SMSCConnectors."+smsc.getName()+".rangeOfAddress",smsc.getRangeOfAddress());
        else
          cfg.removeParam("InfoSme.SMSCConnectors."+smsc.getName()+".rangeOfAddress");
        if (smsc.getInterfaceVersion() >= 0)
          cfg.setInt("InfoSme.SMSCConnectors."+smsc.getName()+".interfaceVersion",smsc.getInterfaceVersion());
        else
          cfg.removeParam("InfoSme.SMSCConnectors."+smsc.getName()+".interfaceVersion");
      }

      cfg.setInt("InfoSme.tasksSwitchTimeout", tasksSwitchTimeout);
      cfg.setString("InfoSme.tasksTablesPrefix", tasksTaskTablesPrefix);

      cfg.setString("InfoSme.storeLocation", storeLocation);
      cfg.setString("InfoSme.archiveLocation", archiveLocation);
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

  public Map getSmscConns() {
    return new HashMap(smscConns);
  }

  public SmscConnector getDefaultSmsc() {
    return defSmscConn;
  }

  public SmscConnector getSmscConn(String name) {
    return (SmscConnector) smscConns.get(name);
  }

  public void setSmscConn(Collection smscs, String def) throws AdminException{

    Map newConn = new HashMap();
    Iterator it = smscs.iterator();
    while(it.hasNext()) {
      SmscConnector o = (SmscConnector)it.next();
      if(o != null) {
        newConn.put(o.getName(), o);
      }
    }
    SmscConnector dS = (SmscConnector)newConn.get(def);
    if(dS == null) {
      throw new AdminException("Default smsc not found with name="+ defSmscConn.getName());
    }
    this.smscConns = new HashMap(newConn);
    this.defSmscConn = dS;
    setOptionsModified(true);
  }

  public static class SmscConnector {
    private String name;
    private String host;
    private int port;
    private String sid;
    private int timeout;
    private String password;


    private String systemType;
    private String rangeOfAddress;
    private int interfaceVersion;

    public String getName() {
      return name;
    }

    public void setName(String name) {
      this.name = name;
    }

    public String getHost() {
      return host;
    }

    public void setHost(String host) {
      this.host = host;
    }

    public int getPort() {
      return port;
    }

    public void setPort(int port) {
      this.port = port;
    }

    public String getSid() {
      return sid;
    }

    public void setSid(String sid) {
      this.sid = sid;
    }

    public int getTimeout() {
      return timeout;
    }

    public void setTimeout(int timeout) {
      this.timeout = timeout;
    }

    public String getPassword() {
      return password;
    }

    public void setPassword(String password) {
      this.password = password;
    }

    public String getSystemType() {
      return systemType;
    }

    public void setSystemType(String systemType) {
      this.systemType = systemType;
    }

    public String getRangeOfAddress() {
      return rangeOfAddress;
    }

    public void setRangeOfAddress(String rangeOfAddress) {
      this.rangeOfAddress = rangeOfAddress;
    }

    public int getInterfaceVersion() {
      return interfaceVersion;
    }

    public void setInterfaceVersion(int interfaceVersion) {
      this.interfaceVersion = interfaceVersion;
    }
  }

  public String getSiebelJDBCSource() {
    return siebelJDBCSource;
  }

  public void setSiebelJDBCSource(String siebelJDBCSource) {
    this.siebelJDBCSource = siebelJDBCSource;
    setOptionsModified(true);
  }

  public String getSiebelJDBCDriver() {
    return siebelJDBCDriver;
  }

  public void setSiebelJDBCDriver(String siebelJDBCDriver) {
    this.siebelJDBCDriver = siebelJDBCDriver;
    setOptionsModified(true);
  }

  public String getSiebelJDBCUser() {
    return siebelJDBCUser;
  }

  public void setSiebelJDBCUser(String siebelJDBCUser) {
    this.siebelJDBCUser = siebelJDBCUser;
    setOptionsModified(true);
  }

  public String getSiebelJDBCPass() {
    return siebelJDBCPass;
  }

  public void setSiebelJDBCPass(String siebelJDBCPass) {
    this.siebelJDBCPass = siebelJDBCPass;
    setOptionsModified(true);
  }

  public int getSiebelTMPeriod() {
    return siebelTMPeriod;
  }

  public void setSiebelTMPeriod(int siebelTMPeriod) {
    this.siebelTMPeriod = siebelTMPeriod;
    setOptionsModified(true);
  }

  public boolean isSiebelTMRemove() {
    return siebelTMRemove;
  }

  public void setSiebelTMRemove(boolean siebelTMRemove) {
    this.siebelTMRemove = siebelTMRemove;
    setOptionsModified(true);
  }


  public boolean isSiebelTReplaceMessage() {
    return siebelTReplaceMessage;
  }

  public void setSiebelTReplaceMessage(boolean siebelTReplaceMessage) {
    this.siebelTReplaceMessage = siebelTReplaceMessage;
    setOptionsModified(true);
  }

  public String getSiebelTSvcType() {
    return siebelTSvcType;
  }

  public void setSiebelTSvcType(String siebelTSvcType) {
    this.siebelTSvcType = siebelTSvcType;
    setOptionsModified(true);
  }

  public boolean isSiebelTRetryOnFail() {
    return siebelTRetryOnFail;
  }

  public void setSiebelTRetryOnFail(boolean siebelTRetryOnFail) {
    this.siebelTRetryOnFail = siebelTRetryOnFail;
    setOptionsModified(true);
  }

  public String getSiebelTRetryPolicy() {
    return siebelTRetryPolicy;
  }

  public void setSiebelTRetryPolicy(String siebelTRetryPolicy) {
    this.siebelTRetryPolicy = siebelTRetryPolicy;
    setOptionsModified(true);
  }

  public Date getSiebelTPeriodStart() {
    return siebelTPeriodStart;
  }

  public void setSiebelTPeriodStart(Date siebelTPeriodStart) {
    this.siebelTPeriodStart = siebelTPeriodStart;
    setOptionsModified(true);
  }

  public Date getSiebelTPeriodEnd() {
    return siebelTPeriodEnd;
  }

  public void setSiebelTPeriodEnd(Date siebelTPeriodEnd) {
    this.siebelTPeriodEnd = siebelTPeriodEnd;
    setOptionsModified(true);
  }

  public Collection getSiebelTWeekDaysSet() {
    return siebelTWeekDaysSet;
  }

  public void setSiebelTWeekDaysSet(Collection siebelTWeekDaysSet) {
    this.siebelTWeekDaysSet = siebelTWeekDaysSet;
    setOptionsModified(true);
  }

  public int getSiebelTCacheSize() {
    return siebelTCacheSize;
  }

  public void setSiebelTCacheSize(int siebelTCacheSize) {
    this.siebelTCacheSize = siebelTCacheSize;
    setOptionsModified(true);
  }

  public int getSiebelTCacheSleep() {
    return siebelTCacheSleep;
  }

  public void setSiebelTCacheSleep(int siebelTCacheSleep) {
    this.siebelTCacheSleep = siebelTCacheSleep;
    setOptionsModified(true);
  }

  public boolean isSiebelTTrMode() {
    return siebelTTrMode;
  }

  public void setSiebelTTrMode(boolean siebelTTrMode) {
    this.siebelTTrMode = siebelTTrMode;
    setOptionsModified(true);
  }

  public boolean isSiebelTKeepHistory() {
    return siebelTKeepHistory;
  }

  public void setSiebelTKeepHistory(boolean siebelTKeepHistory) {
    this.siebelTKeepHistory = siebelTKeepHistory;
    setOptionsModified(true);
  }

  public int getSiebelTUncommitGeneration() {
    return siebelTUncommitGeneration;
  }

  public void setSiebelTUncommitGeneration(int siebelTUncommitGeneration) {
    this.siebelTUncommitGeneration = siebelTUncommitGeneration;
    setOptionsModified(true);
  }

  public int getSiebelTUncommitProcess() {
    return siebelTUncommitProcess;
  }

  public void setSiebelTUncommitProcess(int siebelTUncommitProcess) {
    this.siebelTUncommitProcess = siebelTUncommitProcess;
    setOptionsModified(true);
  }

  public boolean isSiebelTTrackIntegrity() {
    return siebelTTrackIntegrity;
  }

  public void setSiebelTTrackIntegrity(boolean siebelTTrackIntegrity) {
    this.siebelTTrackIntegrity = siebelTTrackIntegrity;
    setOptionsModified(true);
  }

  public String getArchiveLocation() {
    return archiveLocation;
  }

  public void setArchiveLocation(String archiveLocation) {
    this.archiveLocation = archiveLocation;
  }

  public Boolean getUssdPushFeature() {
    return ussdPushFeature;
  }
}
