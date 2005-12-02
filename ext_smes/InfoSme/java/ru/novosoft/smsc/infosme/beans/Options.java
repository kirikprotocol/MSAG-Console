package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.util.SortedList;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:43:24 PM
 */
public class Options extends InfoSmeBean
{
  private String address = "";
  private String svcType = "";
  private int protocolId = 0;
  private int maxMessagesPerSecond = 10;
  private int unrespondedMessagesMax = 0;
  private int unrespondedMessagesSleep = 0;
  private String responceWaitTime = "";
  private String receiptWaitTime = "";
  private String tasksTaskTablesPrefix = "";
  private int tasksSwitchTimeout = 0;

  private int tasksThreadPoolMax = 0;
  private int tasksThreadPoolInit = 0;

  private int eventsThreadPoolMax = 0;
  private int eventsThreadPoolInit = 0;

  private String adminHost = "";
  private int adminPort = 0;

  private String smscHost = "";
  private int smscPort = 0;
  private String smscSid = "";
  private int smscTimeout = 0;
  private String smscPassword = "";


  private String systemDataSourceType = "";
  private int systemDataSourceConnections = 0;
  private String systemDataSourceDbInstance = "";
  private String systemDataSourceDbUserName = "";
  private String systemDataSourceDbUserPassword = "";
  private boolean systemDataSourceWatchdog = false;
  private String systemDataSourceJdbcDriver = "";
  private String systemDataSourceJdbcSource = "";


  private boolean initialized = false;

  private String mbDone = null;
  private String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      try {
        address = getConfig().getString("InfoSme.Address");
        svcType = getConfig().getString("InfoSme.SvcType");
        protocolId = getConfig().getInt("InfoSme.ProtocolId");
        maxMessagesPerSecond = getConfig().getInt("InfoSme.maxMessagesPerSecond");
        unrespondedMessagesMax = getConfig().getInt("InfoSme.unrespondedMessagesMax");
        unrespondedMessagesSleep = getConfig().getInt("InfoSme.unrespondedMessagesSleep");
        responceWaitTime = getConfig().getString("InfoSme.responceWaitTime");
        receiptWaitTime = getConfig().getString("InfoSme.receiptWaitTime");

        tasksThreadPoolMax = getConfig().getInt("InfoSme.TasksThreadPool.max");
        tasksThreadPoolInit = getConfig().getInt("InfoSme.TasksThreadPool.init");

        eventsThreadPoolMax = getConfig().getInt("InfoSme.EventsThreadPool.max");
        eventsThreadPoolInit = getConfig().getInt("InfoSme.EventsThreadPool.init");

        adminHost = getConfig().getString("InfoSme.Admin.host");
        adminPort = getConfig().getInt("InfoSme.Admin.port");

        smscHost = getConfig().getString("InfoSme.SMSC.host");
        smscPort = getConfig().getInt("InfoSme.SMSC.port");
        smscSid = getConfig().getString("InfoSme.SMSC.sid");
        smscTimeout = getConfig().getInt("InfoSme.SMSC.timeout");
        smscPassword = getConfig().getString("InfoSme.SMSC.password");
        tasksSwitchTimeout = getConfig().getInt("InfoSme.tasksSwitchTimeout");
        tasksTaskTablesPrefix = getConfig().getString("InfoSme.tasksTablesPrefix");

        systemDataSourceType = getConfig().getString("InfoSme.systemDataSource.type");
        systemDataSourceConnections = getConfig().getInt("InfoSme.systemDataSource.connections");
        systemDataSourceDbInstance = getConfig().getString("InfoSme.systemDataSource.dbInstance");
        systemDataSourceDbUserName = getConfig().getString("InfoSme.systemDataSource.dbUserName");
        systemDataSourceDbUserPassword = getConfig().getString("InfoSme.systemDataSource.dbUserPassword");
        systemDataSourceWatchdog = getConfig().getBool("InfoSme.systemDataSource.watchdog");
        systemDataSourceJdbcDriver = getConfig().getString("InfoSme.systemDataSource.jdbc.driver");
        systemDataSourceJdbcSource = getConfig().getString("InfoSme.systemDataSource.jdbc.source");
      } catch (Exception e) {
        checkForNulls();
        logger.error(e);
        return error("infosme.error.config_param", e.getMessage());
      }
    }
    checkForNulls();
    return result;
  }

  private void checkForNulls()
  {
/*
    if (address == null) address = "";
    if (svcType == null) svcType = "";
    if (responceWaitTime == null) responceWaitTime = "";
    if (receiptWaitTime == null) receiptWaitTime = "";
    if (tasksTaskTablesPrefix == null) tasksTaskTablesPrefix = "";
    if (adminHost == null) adminHost = "";
    if (smscHost == null) smscHost = "";
    if (smscSid == null) smscSid = "";
    if (smscPassword == null) smscPassword = "";
    if (systemDataSourceType == null) systemDataSourceType = "";
    if (systemDataSourceDbInstance == null) systemDataSourceDbInstance = "";
    if (systemDataSourceDbUserName == null) systemDataSourceDbUserName = "";
    if (systemDataSourceDbUserPassword == null) systemDataSourceDbUserPassword = "";
    if (systemDataSourceJdbcDriver == null) systemDataSourceJdbcDriver = "";
    if (systemDataSourceJdbcSource == null) systemDataSourceJdbcSource = "";
*/
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbDone != null)
      return save();
    if (mbCancel != null)
      return RESULT_DONE;

    return result;
  }

  private int save()
  {
    getConfig().setString("InfoSme.Address", address);
    getConfig().setString("InfoSme.SvcType", svcType);
    getConfig().setInt("InfoSme.ProtocolId", protocolId);
    getConfig().setInt("InfoSme.maxMessagesPerSecond", maxMessagesPerSecond);
    getConfig().setInt("InfoSme.unrespondedMessagesMax", unrespondedMessagesMax);
    getConfig().setInt("InfoSme.unrespondedMessagesSleep", unrespondedMessagesSleep);
    getConfig().setString("InfoSme.responceWaitTime", responceWaitTime);
    getConfig().setString("InfoSme.receiptWaitTime", receiptWaitTime);

    getConfig().setInt("InfoSme.TasksThreadPool.max", tasksThreadPoolMax);
    getConfig().setInt("InfoSme.TasksThreadPool.init", tasksThreadPoolInit);

    getConfig().setInt("InfoSme.EventsThreadPool.max", eventsThreadPoolMax);
    getConfig().setInt("InfoSme.EventsThreadPool.init", eventsThreadPoolInit);

    getConfig().setString("InfoSme.Admin.host", adminHost);
    getConfig().setInt("InfoSme.Admin.port", adminPort);

    getConfig().setString("InfoSme.SMSC.host", smscHost);
    getConfig().setInt("InfoSme.SMSC.port", smscPort);
    getConfig().setString("InfoSme.SMSC.sid", smscSid);
    getConfig().setInt("InfoSme.SMSC.timeout", smscTimeout);
    getConfig().setString("InfoSme.SMSC.password", smscPassword);
    getConfig().setInt("InfoSme.tasksSwitchTimeout", tasksSwitchTimeout);
    getConfig().setString("InfoSme.tasksTablesPrefix", tasksTaskTablesPrefix);

    getConfig().setString("InfoSme.systemDataSource.type", systemDataSourceType);
    getConfig().setInt("InfoSme.systemDataSource.connections", systemDataSourceConnections);
    getConfig().setString("InfoSme.systemDataSource.dbInstance", systemDataSourceDbInstance);
    getConfig().setString("InfoSme.systemDataSource.dbUserName", systemDataSourceDbUserName);
    getConfig().setString("InfoSme.systemDataSource.dbUserPassword", systemDataSourceDbUserPassword);
    getConfig().setBool("InfoSme.systemDataSource.watchdog", systemDataSourceWatchdog);
    getConfig().setString("InfoSme.systemDataSource.jdbc.driver", systemDataSourceJdbcDriver);
    getConfig().setString("InfoSme.systemDataSource.jdbc.source", systemDataSourceJdbcSource);
    getInfoSmeContext().setChangedOptions(true);
    return RESULT_DONE;
  }

  public String getSvcType() {
    return svcType;
  }
  public void setSvcType(String svcType) {
    this.svcType = svcType;
  }
  public int getProtocolIdInt() {
    return protocolId;
  }
  public void setProtocolIdInt(int protocolId) {
    this.protocolId = protocolId;
  }
  public String getProtocolId() {
    return String.valueOf(protocolId);
  }
  public void setProtocolId(String protocolId)
  {
    try {
      this.protocolId = Integer.decode(protocolId).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid InfoSme.ProtocolId parameter value: \"" + protocolId + '"', e);
      this.protocolId = 0;
    }
  }

  public String getSmscHost() {
    return smscHost;
  }
  public void setSmscHost(String smscHost) {
    this.smscHost = smscHost;
  }
  public int getSmscPortInt() {
    return smscPort;
  }
  public void setSmscPortInt(int smscPort) {
    this.smscPort = smscPort;
  }
  public String getSmscPort() {
    return String.valueOf(smscPort);
  }
  public void setSmscPort(String smscPort) {
    try {
      this.smscPort = Integer.decode(smscPort).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid InfoSme.SMSC.port parameter value: \"" + smscPort + '"', e);
      this.smscPort = 0;
    }
  }
  public String getSmscSid() {
    return smscSid;
  }
  public void setSmscSid(String smscSid) {
    this.smscSid = smscSid;
  }
  public int getSmscTimeoutInt() {
    return smscTimeout;
  }
  public void setSmscTimeoutInt(int smscTimeout) {
    this.smscTimeout = smscTimeout;
  }
  public String getSmscTimeout() {
    return String.valueOf(smscTimeout);
  }
  public void setSmscTimeout(String smscTimeout) {
    try {
      this.smscTimeout = Integer.valueOf(smscTimeout).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid InfoSme.SMSC.timeout parameter value: \"" + smscTimeout + '"', e);
      this.smscTimeout = 0;
    }
  }
  public String getSmscPassword() {
    return smscPassword;
  }
  public void setSmscPassword(String smscPassword) {
    this.smscPassword = smscPassword;
  }

  public boolean isInitialized() {
    return initialized;
  }
  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
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

  public int getTasksSwitchTimeoutInt() {
    return tasksSwitchTimeout;
  }
  public void setTasksSwitchTimeoutInt(int tasksSwitchTimeout) {
    this.tasksSwitchTimeout = tasksSwitchTimeout;
  }
  public String getTasksSwitchTimeout() {
    return String.valueOf(tasksSwitchTimeout);
  }
  public void setTasksSwitchTimeout(String tasksSwitchTimeout) {
    try {
      this.tasksSwitchTimeout = Integer.decode(tasksSwitchTimeout).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid int InfoSme.Tasks.switchTimeout parameter value: \"" + smscTimeout + '"', e);
      this.tasksSwitchTimeout = 0;
    }
  }

  public String getTasksTaskTablesPrefix() {
    return tasksTaskTablesPrefix;
  }
  public void setTasksTaskTablesPrefix(String tasksTaskTablesPrefix) {
    this.tasksTaskTablesPrefix = tasksTaskTablesPrefix;
  }

  public Collection getAllDataProviders() {
    return new SortedList(getConfig().getSectionChildShortSectionNames("InfoSme.DataProvider"));
  }

  public String getAddress() {
    return address;
  }
  public void setAddress(String address) {
    this.address = address;
  }

  public int getMaxMessagesPerSecondInt() {
    return maxMessagesPerSecond;
  }
  public void setMaxMessagesPerSecondInt(int maxMessagesPerSecond) {
    this.maxMessagesPerSecond = maxMessagesPerSecond;
  }
  public String getMaxMessagesPerSecond() {
    return String.valueOf(maxMessagesPerSecond);
  }
  public void setMaxMessagesPerSecond(String maxMessagesPerSecond) {
    try {
      this.maxMessagesPerSecond = Integer.decode(maxMessagesPerSecond).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int InfoSme.maxMessagesPerSecond parameter value: " + maxMessagesPerSecond + '"', e);
    }
  }

  public int getUnrespondedMessagesMaxInt() {
    return unrespondedMessagesMax;
  }
  public void setUnrespondedMessagesMaxInt(int unrespondedMessagesMax) {
    this.unrespondedMessagesMax = unrespondedMessagesMax;
  }
  public String getUnrespondedMessagesMax() {
    return String.valueOf(unrespondedMessagesMax);
  }
  public void setUnrespondedMessagesMax(String unrespondedMessagesMax) {
    try {
      this.unrespondedMessagesMax = Integer.decode(unrespondedMessagesMax).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int InfoSme.unrespondedMessagesMax parameter value: " + unrespondedMessagesMax + '"', e);
    }
  }

  public int getUnrespondedMessagesSleepInt() {
    return unrespondedMessagesSleep;
  }
  public void setUnrespondedMessagesSleepInt(int unrespondedMessagesSleep) {
    this.unrespondedMessagesSleep = unrespondedMessagesSleep;
  }
  public String getUnrespondedMessagesSleep() {
    return String.valueOf(unrespondedMessagesSleep);
  }
  public void setUnrespondedMessagesSleep(String unrespondedMessagesSleep) {
    try {
      this.unrespondedMessagesSleep = Integer.decode(unrespondedMessagesSleep).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int InfoSme.unrespondedMessagesSleep parameter value: " + unrespondedMessagesSleep + '"', e);
    }
  }

  public String getResponceWaitTime() {
    return responceWaitTime;
  }
  public void setResponceWaitTime(String responceWaitTime) {
    this.responceWaitTime = responceWaitTime;
  }

  public String getReceiptWaitTime() {
    return receiptWaitTime;
  }
  public void setReceiptWaitTime(String receiptWaitTime) {
    this.receiptWaitTime = receiptWaitTime;
  }

  public int getTasksThreadPoolMaxInt() {
    return tasksThreadPoolMax;
  }
  public void setTasksThreadPoolMaxInt(int tasksThreadPoolMax) {
    this.tasksThreadPoolMax = tasksThreadPoolMax;
  }
  public int getTasksThreadPoolInitInt() {
    return tasksThreadPoolInit;
  }
  public void setTasksThreadPoolInitInt(int tasksThreadPoolInit) {
    this.tasksThreadPoolInit = tasksThreadPoolInit;
  }

  public int getEventsThreadPoolMaxInt() {
    return eventsThreadPoolMax;
  }
  public void setEventsThreadPoolMaxInt(int eventsThreadPoolMax) {
    this.eventsThreadPoolMax = eventsThreadPoolMax;
  }
  public int getEventsThreadPoolInitInt() {
    return eventsThreadPoolInit;
  }
  public void setEventsThreadPoolInitInt(int eventsThreadPoolInit) {
    this.eventsThreadPoolInit = eventsThreadPoolInit;
  }

  public String getTasksThreadPoolMax() {
    return String.valueOf(tasksThreadPoolMax);
  }
  public void setTasksThreadPoolMax(String tasksThreadPoolMax) {
    try {
      this.tasksThreadPoolMax = Integer.decode(tasksThreadPoolMax).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int InfoSme.TasksThreadPool.max parameter value: " + tasksThreadPoolMax + '"', e);
    }
  }

  public String getTasksThreadPoolInit() {
    return String.valueOf(tasksThreadPoolInit);
  }
  public void setTasksThreadPoolInit(String tasksThreadPoolInit) {
    try {
      this.tasksThreadPoolInit = Integer.decode(tasksThreadPoolInit).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int InfoSme.TasksThreadPool.init parameter value: " + tasksThreadPoolInit + '"', e);
    }
  }

  public String getEventsThreadPoolMax() {
    return String.valueOf(eventsThreadPoolMax);
  }
  public void setEventsThreadPoolMax(String eventsThreadPoolMax) {
    try {
      this.eventsThreadPoolMax = Integer.decode(eventsThreadPoolMax).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int InfoSme.EventsThreadPool.max parameter value: " + eventsThreadPoolMax + '"', e);
    }
  }

  public String getEventsThreadPoolInit() {
    return String.valueOf(eventsThreadPoolInit);
  }
  public void setEventsThreadPoolInit(String eventsThreadPoolInit) {
    try {
      this.eventsThreadPoolInit = Integer.decode(eventsThreadPoolInit).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int InfoSme.EventsThreadPool.init parameter value: " + eventsThreadPoolInit + '"', e);
    }
  }

  public String getAdminHost() {
    return adminHost;
  }
  public void setAdminHost(String adminHost) {
    this.adminHost = adminHost;
  }

  public int getAdminPortInt() {
    return adminPort;
  }
  public void setAdminPortInt(int adminPort) {
    this.adminPort = adminPort;
  }
  public String getAdminPort() {
    return String.valueOf(adminPort);
  }
  public void setAdminPort(String adminPort) {
    try {
      this.adminPort = Integer.decode(adminPort).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int InfoSme.Admin.port parameter value: \"" + adminPort + '"', e);
    }
  }

  public String getSystemDataSourceType() {
    return systemDataSourceType;
  }
  public void setSystemDataSourceType(String systemDataSourceType) {
    this.systemDataSourceType = systemDataSourceType;
  }

  public int getSystemDataSourceConnectionsInt() {
    return systemDataSourceConnections;
  }
  public void setSystemDataSourceConnectionsInt(int systemDataSourceConnections) {
    this.systemDataSourceConnections = systemDataSourceConnections;
  }
  public String getSystemDataSourceConnections() {
    return String.valueOf(systemDataSourceConnections);
  }
  public void setSystemDataSourceConnections(String systemDataSourceConnections) {
    try {
      this.systemDataSourceConnections = Integer.decode(systemDataSourceConnections).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int InfoSme.systemDataSource.connections parameter value: \"" + systemDataSourceConnections + '"', e);
    }
  }

  public String getSystemDataSourceDbInstance() {
    return systemDataSourceDbInstance;
  }
  public void setSystemDataSourceDbInstance(String systemDataSourceDbInstance) {
    this.systemDataSourceDbInstance = systemDataSourceDbInstance;
  }

  public String getSystemDataSourceDbUserName() {
    return systemDataSourceDbUserName;
  }
  public void setSystemDataSourceDbUserName(String systemDataSourceDbUserName) {
    this.systemDataSourceDbUserName = systemDataSourceDbUserName;
  }

  public String getSystemDataSourceDbUserPassword() {
    return systemDataSourceDbUserPassword;
  }
  public void setSystemDataSourceDbUserPassword(String systemDataSourceDbUserPassword) {
    this.systemDataSourceDbUserPassword = systemDataSourceDbUserPassword;
  }

  public boolean isSystemDataSourceWatchdog() {
    return systemDataSourceWatchdog;
  }
  public void setSystemDataSourceWatchdog(boolean systemDataSourceWatchdog) {
    this.systemDataSourceWatchdog = systemDataSourceWatchdog;
  }

  public String getSystemDataSourceJdbcDriver() {
    return systemDataSourceJdbcDriver;
  }
  public void setSystemDataSourceJdbcDriver(String systemDataSourceJdbcDriver) {
    this.systemDataSourceJdbcDriver = systemDataSourceJdbcDriver;
  }

  public String getSystemDataSourceJdbcSource() {
    return systemDataSourceJdbcSource;
  }
  public void setSystemDataSourceJdbcSource(String systemDataSourceJdbcSource) {
    this.systemDataSourceJdbcSource = systemDataSourceJdbcSource;
  }
}
