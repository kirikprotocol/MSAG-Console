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

import java.util.*;

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

  private Map smscConns = new HashMap();

  private SmscConnector defSmscConn;

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

      if(cfg.containsSection("SMSCConnectors")) {
        String defname = cfg.getString("SMSCConnectors.default");
        Set childs = cfg.getSectionChildShortSectionNames("SMSCConnectors");
        Iterator i = childs.iterator();

        Map newSmscs = new HashMap();
        while(i.hasNext()) {
          SmscConnector smsc = new SmscConnector();
          String smscName = (String)i.next();
          smsc.setName(smscName);
          smsc.setHost(cfg.getString("SMSCConnectors."+smscName+".host"));
          smsc.setPort(cfg.getInt("SMSCConnectors."+smscName+".port"));
          smsc.setSid(cfg.getString("SMSCConnectors."+smscName+".sid"));
          smsc.setTimeout(cfg.getInt("SMSCConnectors."+smscName+".timeout"));
          smsc.setPassword(cfg.getString("SMSCConnectors."+smscName+".password"));
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

      if(defSmscConn != null) {
        if(smscConns.get(defSmscConn.getName()) == null) {
          throw new AdminException("Default smsc not found with name="+ defSmscConn.getName());
        }
        cfg.setString("SMSCConnectors.default", defSmscConn.getName());
      } else if(!smscConns.isEmpty()) {
        throw new AdminException("Default smsc not found");
      }

      Iterator i = smscConns.values().iterator();
      while(i.hasNext()){
        SmscConnector smsc = (SmscConnector)i.next();
        cfg.setString("SMSCConnectors."+smsc.getName()+".host",smsc.getHost());
        cfg.setInt("SMSCConnectors."+smsc.getName()+".port",smsc.getPort());
        cfg.setInt("SMSCConnectors."+smsc.getName()+".timeout",smsc.getTimeout());
        cfg.setString("SMSCConnectors."+smsc.getName()+".sid",smsc.getSid());
        cfg.setString("SMSCConnectors."+smsc.getName()+".password",smsc.getPassword());
      }

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
  }
}
