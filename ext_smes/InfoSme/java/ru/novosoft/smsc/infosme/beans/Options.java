package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataSource;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyQuery;
import ru.novosoft.smsc.infosme.backend.tables.retrypolicies.RetryPolicyDataItem;
import ru.novosoft.smsc.admin.AdminException;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.text.SimpleDateFormat;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:43:24 PM
 */
public class Options extends InfoSmeBean
{

  //Siebel

  private String siebelJDBCDriver="";
  private String siebelJDBCSource="";
  private String siebelJDBCUser="";
  private String siebelJDBCPass="";

  private boolean siebelRetryOnFail = false;
  private boolean siebelReplaceMessage = false;
  private String siebelSvcType = "";
  private String siebelActivePeriodStart = "";
  private String siebelActivePeriodEnd = "";
  private String[] siebelActiveWeekDays = new String[0];

  private int siebelMessagesCacheSize = 0;
  private int siebelMessagesCacheSleep = 0;
  private boolean siebelTransactionMode = false;
  private int siebelUncommitedInGeneration = 0;
  private int siebelUncommitedInProcess = 0;
  private boolean siebelTrackIntegrity = false;
  private boolean siebelKeepHistory = false;
  private String siebelRetryPolicy = "";

  private static final SimpleDateFormat tf = new SimpleDateFormat("HH:mm:ss");

  private int siebelTMPeriod = 20;
  private boolean siebelTMRemoveStopped = false;
  //

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
  private String archiveLocation;
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
        archiveLocation = getConfig().getArchiveLocation();
        statStoreLocation = getConfig().getStatStoreLocation();

        siebelJDBCDriver = getConfig().getSiebelJDBCDriver();
        siebelJDBCSource = getConfig().getSiebelJDBCSource();
        siebelJDBCUser = getConfig().getSiebelJDBCUser();
        siebelJDBCPass = getConfig().getSiebelJDBCPass();

        siebelRetryOnFail = getConfig().isSiebelTRetryOnFail();
        siebelReplaceMessage = getConfig().isSiebelTReplaceMessage();
        siebelSvcType = getConfig().getSiebelTSvcType();
        siebelActivePeriodStart = (getConfig().getSiebelTPeriodStart() == null) ? "" : tf.format(getConfig().getSiebelTPeriodStart());
        siebelActivePeriodEnd = (getConfig().getSiebelTPeriodEnd() == null) ? "" : tf.format(getConfig().getSiebelTPeriodEnd());

        siebelActiveWeekDays = new String[getConfig().getSiebelTWeekDaysSet().size()];
        int i=0;
        for (Iterator iter = getConfig().getSiebelTWeekDaysSet().iterator(); iter.hasNext();) {
          siebelActiveWeekDays[i] = (String)iter.next();
          i++;
        }

        siebelMessagesCacheSize = getConfig().getSiebelTCacheSize();
        siebelMessagesCacheSleep = getConfig().getSiebelTCacheSleep();
        siebelTransactionMode = getConfig().isSiebelTTrMode();
        siebelUncommitedInGeneration = getConfig().getSiebelTUncommitGeneration();
        siebelUncommitedInProcess = getConfig().getSiebelTUncommitProcess();
        siebelTrackIntegrity = getConfig().isSiebelTTrackIntegrity();
        siebelKeepHistory = getConfig().isSiebelTKeepHistory();
        siebelRetryPolicy = getConfig().getSiebelTRetryPolicy();

        siebelTMPeriod = getConfig().getSiebelTMPeriod();
        siebelTMRemoveStopped = getConfig().isSiebelTMRemove();

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
      InfoSmeConfig.validateSiebelOptions(
          siebelActivePeriodEnd.trim().length() == 0 ? null : tf.parse(siebelActivePeriodEnd),
          siebelActivePeriodStart.trim().length() == 0 ? null : tf.parse(siebelActivePeriodStart),
          siebelRetryOnFail,
          siebelRetryPolicy
      );

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
      getConfig().setArchiveLocation(archiveLocation);
      getConfig().setStatStoreLocation(statStoreLocation);


      getConfig().setSiebelJDBCDriver(siebelJDBCDriver);
      getConfig().setSiebelJDBCSource(siebelJDBCSource);
      getConfig().setSiebelJDBCUser(siebelJDBCUser);
      getConfig().setSiebelJDBCPass(siebelJDBCPass);
      getConfig().setSiebelTRetryOnFail(siebelRetryOnFail);
      getConfig().setSiebelTReplaceMessage(siebelReplaceMessage);
      getConfig().setSiebelTSvcType(siebelSvcType);
      getConfig().setSiebelTPeriodStart( siebelActivePeriodStart.trim().length() == 0 ? null : tf.parse(siebelActivePeriodStart)) ;
      getConfig().setSiebelTPeriodEnd( siebelActivePeriodEnd.trim().length() == 0 ? null : tf.parse(siebelActivePeriodEnd)) ;
      getConfig().setSiebelTWeekDaysSet(Arrays.asList(siebelActiveWeekDays));

      getConfig().setSiebelTCacheSize(siebelMessagesCacheSize);
      getConfig().setSiebelTCacheSleep(siebelMessagesCacheSleep);
      getConfig().setSiebelTTrMode(siebelTransactionMode);
      getConfig().setSiebelTUncommitGeneration(siebelUncommitedInGeneration);
      getConfig().setSiebelTUncommitProcess(siebelUncommitedInProcess);
      getConfig().setSiebelTTrackIntegrity(siebelTrackIntegrity);
      getConfig().setSiebelTKeepHistory(siebelKeepHistory);
      getConfig().setSiebelTRetryPolicy(siebelRetryPolicy);

      getConfig().setSiebelTMPeriod(siebelTMPeriod);
      getConfig().setSiebelTMRemove(siebelTMRemoveStopped);

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

  public String getArchiveLocation() {
    return archiveLocation;
  }

  public void setArchiveLocation(String archiveLocation) {
    this.archiveLocation = archiveLocation;
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

  public boolean isSiebelTransactionMode() {
    return siebelTransactionMode;
  }

  public void setSiebelTransactionMode(boolean siebelTransactionMode) {
    this.siebelTransactionMode = siebelTransactionMode;
  }

  public String getSiebelActivePeriodEnd() {
    return siebelActivePeriodEnd;
  }

  public void setSiebelActivePeriodEnd(String siebelActivePeriodEnd) {
    this.siebelActivePeriodEnd = siebelActivePeriodEnd;
  }

  public String getSiebelActivePeriodStart() {
    return siebelActivePeriodStart;
  }

  public void setSiebelActivePeriodStart(String siebelActivePeriodStart) {
    this.siebelActivePeriodStart = siebelActivePeriodStart;
  }

  public String[] getSiebelActiveWeekDays() {
    return siebelActiveWeekDays;
  }
  public void setSiebelActiveWeekDays(String[] activeWeekDays) {
    this.siebelActiveWeekDays = activeWeekDays;
  }

  public boolean isWeekDayActive(String weekday) {
    for(int i=0; i<siebelActiveWeekDays.length; i++)
      if (siebelActiveWeekDays[i].equals(weekday))
        return true;
    return false;
  }

  public List getRetryPolicies() {
    try{
      QueryResultSet rs = new RetryPolicyDataSource().query(getConfig(), new RetryPolicyQuery(1000, "name", 0));
      List result = new ArrayList(rs.size() + 1);
      for (int i=0; i<rs.size(); i++) {
        RetryPolicyDataItem item = (RetryPolicyDataItem)rs.get(i);
        result.add(item.getName());
      }
      return result;
    }catch(AdminException e){
      logger.error(e,e);
      return new LinkedList();
    }
  }

  public String getSiebelRetryPolicy() {
    return siebelRetryPolicy;
  }

  public void setSiebelRetryPolicy(String retryPolicy) {
    this.siebelRetryPolicy = retryPolicy;
  }

  public boolean isSiebelRetryOnFail() {
    return siebelRetryOnFail;
  }

  public void setSiebelRetryOnFail(boolean retryOnFail) {
    this.siebelRetryOnFail = retryOnFail;
  }


  public boolean isSiebelReplaceMessage() {
    return siebelReplaceMessage;
  }

  public void setSiebelReplaceMessage(boolean replaceMessage) {
    this.siebelReplaceMessage = replaceMessage;
  }

  public String getSiebelSvcType() {
    return siebelSvcType;
  }

  public void setSiebelSvcType(String svcType) {
    this.siebelSvcType = svcType;
  }

  public String getSiebelMessagesCacheSize() {
    return Integer.toString(siebelMessagesCacheSize);
  }

  public void setSiebelMessagesCacheSize(String siebelMessagesCacheSize) {
    if(siebelMessagesCacheSize != null && siebelMessagesCacheSize.length()>0) {
      try{
        this.siebelMessagesCacheSize = Integer.parseInt(siebelMessagesCacheSize);
      }catch(NumberFormatException e) {
        logger.error(e,e);
      }
    }
  }

  public String getSiebelMessagesCacheSleep() {
    return Integer.toString(siebelMessagesCacheSleep);
  }

  public void setSiebelMessagesCacheSleep(String siebelMessagesCacheSleep) {
    if(siebelMessagesCacheSleep != null && siebelMessagesCacheSleep.length()>0) {
      try{
        this.siebelMessagesCacheSleep = Integer.parseInt(siebelMessagesCacheSleep);
      }catch(NumberFormatException e) {
        logger.error(e,e);
      }
    }
  }

  public String getSiebelUncommitedInGeneration() {
    return Integer.toString(siebelUncommitedInGeneration);
  }

  public void setSiebelUncommitedInGeneration(String siebelUncommitedInGeneration) {
    if(siebelUncommitedInGeneration != null && siebelUncommitedInGeneration.length()>0) {
      try{
        this.siebelUncommitedInGeneration = Integer.parseInt(siebelUncommitedInGeneration);
      }catch(NumberFormatException e) {
        logger.error(e,e);
      }
    }
  }

  public String getSiebelUncommitedInProcess() {
    return Integer.toString(siebelUncommitedInProcess);
  }

  public void setSiebelUncommitedInProcess(String siebelUncommitedInProcess) {
    if(siebelUncommitedInProcess != null && siebelUncommitedInProcess.length()>0) {
      try{
        this.siebelUncommitedInProcess = Integer.parseInt(siebelUncommitedInProcess);
      }catch(NumberFormatException e) {
        logger.error(e,e);
      }
    }
  }

  public boolean isSiebelTrackIntegrity() {
    return siebelTrackIntegrity;
  }

  public void setSiebelTrackIntegrity(boolean siebelTrackIntegrity) {
    this.siebelTrackIntegrity = siebelTrackIntegrity;
  }

  public boolean isSiebelKeepHistory() {
    return siebelKeepHistory;
  }

  public void setSiebelKeepHistory(boolean siebelKeepHistory) {
    this.siebelKeepHistory = siebelKeepHistory;
  }

  public String getSiebelJDBCDriver() {
    return siebelJDBCDriver;
  }

  public void setSiebelJDBCDriver(String siebelJDBCDriver) {
    this.siebelJDBCDriver = siebelJDBCDriver;
  }

  public String getSiebelJDBCSource() {
    return siebelJDBCSource;
  }

  public void setSiebelJDBCSource(String siebelJDBCSource) {
    this.siebelJDBCSource = siebelJDBCSource;
  }

  public String getSiebelJDBCUser() {
    return siebelJDBCUser;
  }

  public void setSiebelJDBCUser(String siebelJDBCUser) {
    this.siebelJDBCUser = siebelJDBCUser;
  }

  public String getSiebelJDBCPass() {
    return siebelJDBCPass;
  }

  public void setSiebelJDBCPass(String siebelJDBCPass) {
    this.siebelJDBCPass = siebelJDBCPass;
  }

  public String getSiebelTMPeriod() {
    return Integer.toString(siebelTMPeriod);
  }

  public void setSiebelTMPeriod(String siebelTMPeriod) {
    if(siebelTMPeriod != null && siebelTMPeriod.trim().length()>0) {
      try{
        this.siebelTMPeriod = Integer.parseInt(siebelTMPeriod.trim());
      }catch(NumberFormatException e) {
        logger.error(e,e);
      }
    }
  }

  public boolean isSiebelTMRemoveStopped() {
    return siebelTMRemoveStopped;
  }

  public void setSiebelTMRemoveStopped(boolean siebelRemoveStopped) {
    this.siebelTMRemoveStopped = siebelRemoveStopped;
  }
}