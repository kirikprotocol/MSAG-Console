package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.SortedList;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Collection;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 18:37:43
 * To change this template use Options | File Templates.
 */
public class Options extends MCISmeBean
{
  private String address = "";
  private String svcType = "";
  private String callingMask = "";
  private String calledMask = "";
  private int protocolId = 0;
  private int daysValid = 1;
  private int unrespondedMessagesMax = 0;
  private int unrespondedMessagesSleep = 0;
  private String responceWaitTime = "";
  private String receiptWaitTime = "";
  private int inputQueueSize=0;
  private int outputQueueSize=0;
  private int maxRowsPerMessage=0;
  private boolean forceInform = false;
  private boolean forceNotify = false;

  private int smppThreadPoolMax = 0;
  private int smppThreadPoolInit = 0;

  private String adminHost = "";
  private int adminPort = 0;

  private String smscHost = "";
  private int smscPort = 0;
  private String smscSid = "";
  private int smscTimeout = 0;
  private String smscPassword = "";

  private int HSN = 0;
  private int SPN = 0;
  private String TSM = "";

  private int causeBusy          = 0;
  private int causeNoReply       = 0;
  private int causeUnconditional = 0;
  private int causeAbsent        = 0;
  private int causeOther         = 0;

  private boolean informBusy          = false;
  private boolean informNoReply       = false;
  private boolean informUnconditional = false;
  private boolean informAbsent        = false;
  private boolean informOther         = false;

  private String  dataSourceType = "";
  private int     dataSourceConnections = 0;
  private String  dataSourceDbInstance = "";
  private String  dataSourceDbUserName = "";
  private String  dataSourceDbUserPassword = "";
  private boolean dataSourceWatchdog = false;
  private String  dataSourceJdbcDriver = "";
  private String  dataSourceJdbcSource = "";

  private boolean initialized = false;

  private String mbDone = null;
  private String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)  return result;

    if (!initialized) {
      try {
        address = getConfig().getString("MCISme.Address");
        svcType = getConfig().getString("MCISme.SvcType");
        protocolId = getConfig().getInt("MCISme.ProtocolId");
        callingMask = getConfig().getString("MCISme.CallingMask");
        calledMask  = getConfig().getString("MCISme.CalledMask");
        unrespondedMessagesMax = getConfig().getInt("MCISme.unrespondedMessagesMax");
        unrespondedMessagesSleep = getConfig().getInt("MCISme.unrespondedMessagesSleep");
        responceWaitTime = getConfig().getString("MCISme.responceWaitTime");
        receiptWaitTime = getConfig().getString("MCISme.receiptWaitTime");
        inputQueueSize = getConfig().getInt("MCISme.inputQueueSize");
        outputQueueSize = getConfig().getInt("MCISme.outputQueueSize");
        maxRowsPerMessage = getConfig().getInt("MCISme.maxRowsPerMessage");
        forceInform = getConfig().getBool("MCISme.forceInform");
        forceNotify = getConfig().getBool("MCISme.forceNotify");

        smppThreadPoolMax = getConfig().getInt("MCISme.SMPPThreadPool.max");
        smppThreadPoolInit = getConfig().getInt("MCISme.SMPPThreadPool.init");

        adminHost = getConfig().getString("MCISme.Admin.host");
        adminPort = getConfig().getInt("MCISme.Admin.port");

        smscHost = getConfig().getString("MCISme.SMSC.host");
        smscPort = getConfig().getInt("MCISme.SMSC.port");
        smscSid = getConfig().getString("MCISme.SMSC.sid");
        smscTimeout = getConfig().getInt("MCISme.SMSC.timeout");
        smscPassword = getConfig().getString("MCISme.SMSC.password");

        HSN = getConfig().getInt("MCISme.Circuits.hsn");
        SPN = getConfig().getInt("MCISme.Circuits.spn");
        TSM = getConfig().getString("MCISme.Circuits.tsm");

        causeBusy  = getConfig().getInt ("MCISme.Reasons.Busy.cause");
        informBusy = getConfig().getBool("MCISme.Reasons.Busy.inform");
        causeNoReply  = getConfig().getInt ("MCISme.Reasons.NoReply.cause");
        informNoReply = getConfig().getBool("MCISme.Reasons.NoReply.inform");
        causeUnconditional  = getConfig().getInt ("MCISme.Reasons.Unconditional.cause");
        informUnconditional = getConfig().getBool("MCISme.Reasons.Unconditional.inform");
        causeAbsent  = getConfig().getInt ("MCISme.Reasons.Absent.cause");
        informAbsent = getConfig().getBool("MCISme.Reasons.Absent.inform");
        causeOther   = getConfig().getInt ("MCISme.Reasons.Other.cause");
        informOther  = getConfig().getBool("MCISme.Reasons.Other.inform");

        dataSourceType = getConfig().getString("MCISme.DataSource.type");
        dataSourceConnections = getConfig().getInt("MCISme.DataSource.connections");
        dataSourceDbInstance = getConfig().getString("MCISme.DataSource.dbInstance");
        dataSourceDbUserName = getConfig().getString("MCISme.DataSource.dbUserName");
        dataSourceDbUserPassword = getConfig().getString("MCISme.DataSource.dbUserPassword");
        dataSourceWatchdog = getConfig().getBool("MCISme.DataSource.watchdog");
        dataSourceJdbcDriver = getConfig().getString("MCISme.DataSource.jdbc.driver");
        dataSourceJdbcSource = getConfig().getString("MCISme.DataSource.jdbc.source");
      } catch (Exception e) {
        logger.error(e);
        return error(e.getMessage());
      }
    }
    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)  return result;

    if (mbDone != null)   return save();
    if (mbCancel != null) return RESULT_DONE;

    return result;
  }

  private int save()
  {
    getConfig().setString("MCISme.Address", address);
    getConfig().setString("MCISme.SvcType", svcType);
    getConfig().setInt   ("MCISme.ProtocolId",  protocolId);
    getConfig().setString("MCISme.CallingMask", callingMask);
    getConfig().setString("MCISme.CalledMask" , calledMask);
    getConfig().setInt   ("MCISme.unrespondedMessagesMax", unrespondedMessagesMax);
    getConfig().setInt   ("MCISme.unrespondedMessagesSleep", unrespondedMessagesSleep);
    getConfig().setString("MCISme.responceWaitTime", responceWaitTime);
    getConfig().setString("MCISme.receiptWaitTime", receiptWaitTime);
    getConfig().setInt   ("MCISme.inputQueueSize", inputQueueSize);
    getConfig().setInt   ("MCISme.outputQueueSize", outputQueueSize);
    getConfig().setInt   ("MCISme.maxRowsPerMessage", maxRowsPerMessage);
    getConfig().setBool  ("MCISme.forceInform", forceInform);
    getConfig().setBool  ("MCISme.forceNotify", forceNotify);

    getConfig().setInt   ("MCISme.SMPPThreadPool.max", smppThreadPoolMax);
    getConfig().setInt   ("MCISme.SMPPThreadPool.init", smppThreadPoolInit);

    getConfig().setString("MCISme.Admin.host", adminHost);
    getConfig().setInt   ("MCISme.Admin.port", adminPort);

    getConfig().setString("MCISme.SMSC.host", smscHost);
    getConfig().setInt   ("MCISme.SMSC.port", smscPort);
    getConfig().setString("MCISme.SMSC.sid", smscSid);
    getConfig().setInt   ("MCISme.SMSC.timeout", smscTimeout);
    getConfig().setString("MCISme.SMSC.password", smscPassword);

    getConfig().setInt   ("MCISme.Circuits.hsn", HSN);
    getConfig().setInt   ("MCISme.Circuits.spn", SPN);
    getConfig().setString("MCISme.Circuits.tsm", TSM);

    getConfig().setInt   ("MCISme.Reasons.Busy.cause", causeBusy);
    getConfig().setBool  ("MCISme.Reasons.Busy.inform", informBusy);
    getConfig().setInt   ("MCISme.Reasons.NoReply.cause", causeNoReply);
    getConfig().setBool  ("MCISme.Reasons.NoReply.inform", informNoReply);
    getConfig().setInt   ("MCISme.Reasons.Unconditional.cause", causeUnconditional);
    getConfig().setBool  ("MCISme.Reasons.Unconditional.inform", informUnconditional);
    getConfig().setInt   ("MCISme.Reasons.Absent.cause", causeAbsent);
    getConfig().setBool  ("MCISme.Reasons.Absent.inform", informAbsent);
    getConfig().setInt   ("MCISme.Reasons.Other.cause", causeOther);
    getConfig().setBool  ("MCISme.Reasons.Other.inform", informOther);

    getConfig().setString("MCISme.DataSource.type", dataSourceType);
    getConfig().setInt   ("MCISme.DataSource.connections", dataSourceConnections);
    getConfig().setString("MCISme.DataSource.dbInstance", dataSourceDbInstance);
    getConfig().setString("MCISme.DataSource.dbUserName", dataSourceDbUserName);
    getConfig().setString("MCISme.DataSource.dbUserPassword", dataSourceDbUserPassword);
    getConfig().setBool  ("MCISme.DataSource.watchdog", dataSourceWatchdog);
    getConfig().setString("MCISme.DataSource.jdbc.driver", dataSourceJdbcDriver);
    getConfig().setString("MCISme.DataSource.jdbc.source", dataSourceJdbcSource);

    getMCISmeContext().setChangedOptions(true);
    return RESULT_DONE;
  }

  public String getSvcType() {
    return svcType;
  }
  public void setSvcType(String svcType) {
    this.svcType = svcType;
  }

  public String getCallingMask() {
    return callingMask;
  }
  public void setCallingMask(String callingMask) {
    this.callingMask = callingMask;
  }

  public String getCalledMask() {
    return calledMask;
  }
  public void setCalledMask(String calledMask) {
    this.calledMask = calledMask;
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
      logger.error("Invalid MCISme.ProtocolId parameter value: \"" + protocolId + '"', e);
      this.protocolId = 0;
    }
  }

  public int getDaysValidInt() {
    return daysValid;
  }
  public void setDaysValidInt(int daysValid) {
    this.daysValid = daysValid;
  }
  public String getDaysValid() {
    return String.valueOf(daysValid);
  }
  public void setDaysValid(String daysValid)
  {
    try {
      this.daysValid = Integer.decode(daysValid).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid MCISme.DaysValid parameter value: \"" + daysValid + '"', e);
      this.daysValid = 1;
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
      logger.error("Invalid MCISme.SMSC.port parameter value: \"" + smscPort + '"', e);
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
      logger.error("Invalid MCISme.SMSC.timeout parameter value: \"" + smscTimeout + '"', e);
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

  public Collection getAllDataProviders() {
    return new SortedList(getConfig().getSectionChildShortSectionNames("MCISme.DataProvider"));
  }

  public String getAddress() {
    return address;
  }
  public void setAddress(String address) {
    this.address = address;
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
      logger.debug("Invalid int MCISme.unrespondedMessagesMax parameter value: " + unrespondedMessagesMax + '"', e);
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
  public void setUnrespondedMessagesSleep(String unrespondedMessagesSleep)
  {
    try {
      this.unrespondedMessagesSleep = Integer.decode(unrespondedMessagesSleep).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.unrespondedMessagesSleep parameter value: " + unrespondedMessagesSleep + '"', e);
    }
  }

  public int getInputQueueSizeInt() {
    return inputQueueSize;
  }
  public void setInputQueueSizeInt(int inputQueueSize) {
    this.inputQueueSize = inputQueueSize;
  }
  public String getInputQueueSize() {
    return String.valueOf(inputQueueSize);
  }
  public void setInputQueueSize(String inputQueueSize)
  {
    try {
      this.inputQueueSize = Integer.decode(inputQueueSize).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.inputQueueSize parameter value: " + inputQueueSize + '"', e);
    }
  }

  public int getOutputQueueSizeInt() {
    return outputQueueSize;
  }
  public void setOutputQueueSizeInt(int outputQueueSize) {
    this.outputQueueSize = outputQueueSize;
  }
  public String getOutputQueueSize() {
    return String.valueOf(outputQueueSize);
  }
  public void setOutputQueueSize(String outputQueueSize)
  {
    try {
      this.outputQueueSize = Integer.decode(outputQueueSize).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.outputQueueSize parameter value: " + outputQueueSize + '"', e);
    }
  }

  public int getMaxRowsPerMessageInt() {
    return maxRowsPerMessage;
  }
  public void setMaxRowsPerMessageInt(int maxRowsPerMessage) {
    this.maxRowsPerMessage = maxRowsPerMessage;
  }
  public String getMaxRowsPerMessage() {
    return String.valueOf(maxRowsPerMessage);
  }
  public void setMaxRowsPerMessage(String maxRowsPerMessage)
  {
    try {
      this.maxRowsPerMessage = Integer.decode(maxRowsPerMessage).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.maxRowsPerMessage parameter value: " + maxRowsPerMessage + '"', e);
    }
  }

  public boolean isForceInform() {
    return forceInform;
  }
  public void setForceInform(boolean forceInform) {
    this.forceInform = forceInform;
  }
  public boolean isForceNotify() {
    return forceNotify;
  }
  public void setForceNotify(boolean forceNotify) {
    this.forceNotify = forceNotify;
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

  public int getSmppThreadPoolMaxInt() {
    return smppThreadPoolMax;
  }
  public void setSmppThreadPoolMaxInt(int smppThreadPoolMax) {
    this.smppThreadPoolMax = smppThreadPoolMax;
  }
  public String getSmppThreadPoolMax() {
    return String.valueOf(smppThreadPoolMax);
  }
  public void setSmppThreadPoolMax(String smppThreadPoolMax)
  {
    try {
      this.smppThreadPoolMax = Integer.decode(smppThreadPoolMax).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.TasksThreadPool.max parameter value: " + smppThreadPoolMax + '"', e);
    }
  }

  public int getSmppThreadPoolInitInt() {
    return smppThreadPoolInit;
  }
  public void setSmppThreadPoolInitInt(int smppThreadPoolInit) {
    this.smppThreadPoolInit = smppThreadPoolInit;
  }
  public String getSmppThreadPoolInit() {
    return String.valueOf(smppThreadPoolInit);
  }
  public void setSmppThreadPoolInit(String smppThreadPoolInit)
  {
    try {
      this.smppThreadPoolInit = Integer.decode(smppThreadPoolInit).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.TasksThreadPool.init parameter value: " + smppThreadPoolInit + '"', e);
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
      logger.debug("Invalid int MCISme.Admin.port parameter value: \"" + adminPort + '"', e);
    }
  }

  public String getTSM() {
    return TSM;
  }
  public void setTSM(String TSM) {
    this.TSM = TSM;
    try {
      Integer.parseInt(TSM, 16);
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Circuits.tsm hex parameter value: \"" + TSM + '"', e);
    }
  }
  public int getHSNInt() {
    return HSN;
  }
  public void setHSNInt(int HSN) {
    this.HSN = HSN;
  }
  public String getHSN() {
    return String.valueOf(HSN);
  }
  public void setHSN(String HSN) {
    try {
      this.HSN = Integer.decode(HSN).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Circuits.hsn parameter value: \"" + HSN + '"', e);
    }
  }
  public int getSPNInt() {
    return SPN;
  }
  public void setSPNInt(int SPN) {
    this.SPN = SPN;
  }
  public String getSPN() {
    return String.valueOf(SPN);
  }
  public void setSPN(String SPN) {
    try {
      this.SPN = Integer.decode(SPN).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Circuits.spn parameter value: \"" + SPN + '"', e);
    }
  }

  public int getCauseBusyInt() {
    return causeBusy;
  }
  public void setCauseBusyInt(int causeBusy) {
    this.causeBusy = causeBusy;
  }
  public String getCauseBusy() {
    return String.valueOf(causeBusy);
  }
  public void setCauseBusy(String causeBusy) {
    try { this.causeBusy = Integer.decode(causeBusy).intValue(); } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Reasons.Busy.cause parameter value: \"" + causeBusy + '"', e);
    }
  }
  public boolean isInformBusy() {
    return informBusy;
  }
  public void setInformBusy(boolean informBusy) {
    this.informBusy = informBusy;
  }

  public int getCauseNoReplyInt() {
    return causeNoReply;
  }
  public void setCauseNoReplyInt(int causeNoReply) {
    this.causeNoReply = causeNoReply;
  }
  public String getCauseNoReply() {
    return String.valueOf(causeNoReply);
  }
  public void setCauseNoReply(String causeNoReply) {
    try { this.causeNoReply = Integer.decode(causeNoReply).intValue(); } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Reasons.NoReply.cause parameter value: \"" + causeNoReply + '"', e);
    }
  }
  public boolean isInformNoReply() {
    return informNoReply;
  }
  public void setInformNoReply(boolean informNoReply) {
    this.informNoReply = informNoReply;
  }

  public int getCauseUnconditionalInt() {
    return causeUnconditional;
  }
  public void setCauseUnconditionalInt(int causeUnconditional) {
    this.causeUnconditional = causeUnconditional;
  }
  public String getCauseUnconditional() {
    return String.valueOf(causeUnconditional);
  }
  public void setCauseUnconditional(String causeUnconditional) {
    try { this.causeUnconditional = Integer.decode(causeUnconditional).intValue(); } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Reasons.Unconditional.cause parameter value: \"" + causeUnconditional + '"', e);
    }
  }
  public boolean isInformUnconditional() {
    return informUnconditional;
  }
  public void setInformUnconditional(boolean informUnconditional) {
    this.informUnconditional = informUnconditional;
  }

  public int getCauseAbsentInt() {
    return causeAbsent;
  }
  public void setCauseAbsentInt(int causeAbsent) {
    this.causeAbsent = causeAbsent;
  }
  public String getCauseAbsent() {
    return String.valueOf(causeAbsent);
  }
  public void setCauseAbsent(String causeAbsent) {
    try { this.causeAbsent = Integer.decode(causeAbsent).intValue(); } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Reasons.Absent.cause parameter value: \"" + causeAbsent + '"', e);
    }
  }
  public boolean isInformAbsent() {
    return informAbsent;
  }
  public void setInformAbsent(boolean informAbsent) {
    this.informAbsent = informAbsent;
  }

  public int getCauseOtherInt() {
    return causeOther;
  }
  public void setCauseOtherInt(int causeOther) {
    this.causeOther = causeOther;
  }
  public String getCauseOther() {
    return String.valueOf(causeOther);
  }
  public void setCauseOther(String causeOther) {
    try { this.causeOther = Integer.decode(causeOther).intValue(); } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Reasons.Other.cause parameter value: \"" + causeOther + '"', e);
    }
  }
  public boolean isInformOther() {
    return informOther;
  }
  public void setInformOther(boolean informOther) {
    this.informOther = informOther;
  }

  public String getDataSourceType() {
    return dataSourceType;
  }
  public void setDataSourceType(String dataSourceType) {
    this.dataSourceType = dataSourceType;
  }
  public int getDataSourceConnectionsInt() {
    return dataSourceConnections;
  }
  public void setDataSourceConnectionsInt(int dataSourceConnections) {
    this.dataSourceConnections = dataSourceConnections;
  }
  public String getDataSourceConnections() {
    return String.valueOf(dataSourceConnections);
  }
  public void setDataSourceConnections(String dataSourceConnections)
  {
    try {
      this.dataSourceConnections = Integer.decode(dataSourceConnections).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.DataSource.connections parameter value: \"" + dataSourceConnections + '"', e);
    }
  }
  public String getDataSourceDbInstance() {
    return dataSourceDbInstance;
  }
  public void setDataSourceDbInstance(String dataSourceDbInstance) {
    this.dataSourceDbInstance = dataSourceDbInstance;
  }
  public String getDataSourceDbUserName() {
    return dataSourceDbUserName;
  }
  public void setDataSourceDbUserName(String dataSourceDbUserName) {
    this.dataSourceDbUserName = dataSourceDbUserName;
  }
  public String getDataSourceDbUserPassword() {
    return dataSourceDbUserPassword;
  }
  public void setDataSourceDbUserPassword(String dataSourceDbUserPassword) {
    this.dataSourceDbUserPassword = dataSourceDbUserPassword;
  }
  public boolean isDataSourceWatchdog() {
    return dataSourceWatchdog;
  }
  public void setDataSourceWatchdog(boolean dataSourceWatchdog) {
    this.dataSourceWatchdog = dataSourceWatchdog;
  }
  public String getDataSourceJdbcDriver() {
    return dataSourceJdbcDriver;
  }
  public void setDataSourceJdbcDriver(String dataSourceJdbcDriver) {
    this.dataSourceJdbcDriver = dataSourceJdbcDriver;
  }
  public String getDataSourceJdbcSource() {
    return dataSourceJdbcSource;
  }
  public void setDataSourceJdbcSource(String dataSourceJdbcSource) {
    this.dataSourceJdbcSource = dataSourceJdbcSource;
  }

}
