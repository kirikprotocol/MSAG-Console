package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Map;
import java.util.Iterator;
import java.util.Collection;

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

  private String defSmscConn = "";

  private OptionsSmscHelper smscHelper = new  OptionsSmscHelper("infosme.label.smsc.connectors", "smsc_connectors", 70, true);

  private String storeLocation;
  private String statStoreLocation;

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
        address = getConfig().getAddress();
        svcType = getConfig().getSvcType();
        protocolId = getConfig().getProtocolId();
        maxMessagesPerSecond = getConfig().getMaxMessagesPerSecond();
        unrespondedMessagesMax = getConfig().getUnrespondedMessagesMax();
        unrespondedMessagesSleep = getConfig().getUnrespondedMessagesSleep();
        responceWaitTime = getConfig().getResponceWaitTime();
        receiptWaitTime = getConfig().getReceiptWaitTime();

        smscHelper.fillSmscConn(getConfig().getSmscConns().values());

        if(getConfig().getDefaultSmsc() != null) {
          defSmscConn = getConfig().getDefaultSmsc().getName();
        }

        tasksThreadPoolMax = getConfig().getTasksThreadPoolMax();
        tasksThreadPoolInit = getConfig().getTasksThreadPoolInit();

        eventsThreadPoolMax = getConfig().getEventsThreadPoolMax();
        eventsThreadPoolInit = getConfig().getEventsThreadPoolInit();

        adminHost = getConfig().getAdminHost();
        adminPort = getConfig().getAdminPort();

        tasksSwitchTimeout = getConfig().getTasksSwitchTimeout();
        tasksTaskTablesPrefix = getConfig().getTasksTaskTablesPrefix();

        storeLocation = getConfig().getStoreLocation();
        statStoreLocation = getConfig().getStatStoreLocation();

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
    try{
      getConfig().setAddress(address);
      getConfig().setSvcType(svcType);
      getConfig().setProtocolId(protocolId);
      getConfig().setMaxMessagesPerSecond(maxMessagesPerSecond);
      getConfig().setUnrespondedMessagesMax(unrespondedMessagesMax);
      getConfig().setUnrespondedMessagesSleep(unrespondedMessagesSleep);
      getConfig().setResponceWaitTime(responceWaitTime);
      getConfig().setReceiptWaitTime(receiptWaitTime);

      getConfig().setTasksThreadPoolMax(tasksThreadPoolMax);
      getConfig().setTasksThreadPoolInit(tasksThreadPoolInit);

      getConfig().setEventsThreadPoolMax(eventsThreadPoolMax);
      getConfig().setEventsThreadPoolInit(eventsThreadPoolInit);

      getConfig().setAdminHost(adminHost);
      getConfig().setAdminPort(adminPort);

      Map oldSmscConns = getConfig().getSmscConns();
      Iterator oldIt = oldSmscConns.values().iterator();
      Collection newConn = smscHelper.getSmscConn();

      while(oldIt.hasNext()) {
        InfoSmeConfig.SmscConnector o = (InfoSmeConfig.SmscConnector)oldIt.next();
        Iterator newIt = newConn.iterator();
        boolean remove = true;
        while(newIt.hasNext()){
          InfoSmeConfig.SmscConnector n = (InfoSmeConfig.SmscConnector)newIt.next();
          if(n.getName().equals(o.getName())) {
            remove = false;
            break;
          }
        }
        if(remove) {
          if(!getAppContext().getRegionsManager().getRegionsByInfoSmeSmsc(o.getName()).isEmpty()) {
            smscHelper.clear();
            smscHelper.fillSmscConn(oldSmscConns.values());
            return warning("SmscConnector is used in Regions: "+o.getName());
          }
        }
      }
      getConfig().setSmscConn(smscHelper.getSmscConn(), defSmscConn);
      getConfig().setTasksSwitchTimeout(tasksSwitchTimeout);
      getConfig().setTasksTaskTablesPrefix(tasksTaskTablesPrefix);

      getConfig().setStoreLocation(storeLocation);
      getConfig().setStatStoreLocation(statStoreLocation);

      return RESULT_DONE;

    }catch (Throwable e) {
      return warning(e.getMessage());
    }
  }

  public DynamicTableHelper getSmscHelper() {
    return smscHelper;
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
//
//  public String getSmscHost() {
//    return smscHost;
//  }
//  public void setSmscHost(String smscHost) {
//    this.smscHost = smscHost;
//  }
//  public int getSmscPortInt() {
//    return smscPort;
//  }
//  public void setSmscPortInt(int smscPort) {
//    this.smscPort = smscPort;
//  }
//  public String getSmscPort() {
//    return String.valueOf(smscPort);
//  }
//  public void setSmscPort(String smscPort) {
//    try {
//      this.smscPort = Integer.decode(smscPort).intValue();
//    } catch (NumberFormatException e) {
//      logger.error("Invalid InfoSme.SMSC.port parameter value: \"" + smscPort + '"', e);
//      this.smscPort = 0;
//    }
//  }
//  public String getSmscSid() {
//    return smscSid;
//  }
//  public void setSmscSid(String smscSid) {
//    this.smscSid = smscSid;
//  }
//  public int getSmscTimeoutInt() {
//    return smscTimeout;
//  }
//  public void setSmscTimeoutInt(int smscTimeout) {
//    this.smscTimeout = smscTimeout;
//  }
//  public String getSmscTimeout() {
//    return String.valueOf(smscTimeout);
//  }
//  public void setSmscTimeout(String smscTimeout) {
//    try {
//      this.smscTimeout = Integer.valueOf(smscTimeout).intValue();
//    } catch (NumberFormatException e) {
//      logger.error("Invalid InfoSme.SMSC.timeout parameter value: \"" + smscTimeout + '"', e);
//      this.smscTimeout = 0;
//    }
//  }
//  public String getSmscPassword() {
//    return smscPassword;
//  }
//  public void setSmscPassword(String smscPassword) {
//    this.smscPassword = smscPassword;
//  }

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
      logger.error("Invalid int InfoSme.Tasks.switchTimeout parameter value: \"" + tasksSwitchTimeout + '"', e);
      this.tasksSwitchTimeout = 0;
    }
  }

  public String getTasksTaskTablesPrefix() {
    return tasksTaskTablesPrefix;
  }
  public void setTasksTaskTablesPrefix(String tasksTaskTablesPrefix) {
    this.tasksTaskTablesPrefix = tasksTaskTablesPrefix;
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

  public String getStoreLocation() {
    return storeLocation;
  }

  public void setStoreLocation(String storeLocation) {
    this.storeLocation = storeLocation;
  }

  public String getStatStoreLocation() {
    return statStoreLocation;
  }

  public void setStatStoreLocation(String statStoreLocation) {
    this.statStoreLocation = statStoreLocation;
  }

  public String getDefSmscConn() {
    return defSmscConn;
  }

  public void setDefSmscConn(String defSmscConn) {
    this.defSmscConn = defSmscConn;
  }
}
