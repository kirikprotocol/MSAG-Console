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
  private String redirectionAddress = "";
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
  private boolean defaultInform = false;
  private boolean defaultNotify = false;

  public final static int NO_CONSTRAINT           = 0;
  public final static int MAX_CALLERS_CONSTRAINT  = 10;
  public final static int MAX_MESSAGES_CONSTRAINT = 20;
  private int constraintType  = NO_CONSTRAINT;
  private int constraintValue = -1;

  private int smppThreadPoolMax = 0;
  private int smppThreadPoolInit = 0;

  private String adminHost = "";
  private int adminPort = 0;

  private String smscHost = "";
  private int smscPort = 0;
  private String smscSid = "";
  private int smscTimeout = 0;
  private String smscPassword = "";

  private boolean skipUnknownCaller = false;
  private int releaseStrategy    = RELEASE_REDIRECT_STRATEGY; // MTS default
  private int causeBusy          = 0;
  private int causeNoReply       = 0;
  private int causeUnconditional = 0;
  private int causeAbsent        = 0;
  private int causeDetach        = 0;
  private int causeOther         = 0;

  private boolean informBusy          = false;
  private boolean informNoReply       = false;
  private boolean informUnconditional = false;
  private boolean informAbsent        = false;
  private boolean informDetach        = false;
  private boolean informOther         = false;

  private final static int MASK_NONE    = 0x00;
  private final static int MASK_ABSENT  = 0x01;
  private final static int MASK_BUSY    = 0x02;
  private final static int MASK_NOREPLY = 0x04;
  private final static int MASK_UNCOND  = 0x08;
  private final static int MASK_DETACH  = 0x10;
  private final static int MASK_ALL     = 0xFF;
  private boolean defaultBusy           = false;
  private boolean defaultNoReply        = false;
  private boolean defaultUnconditional  = false;
  private boolean defaultAbsent         = false;
  private boolean defaultDetach         = false;

  private String  dataSourceType = "";
  private int     dataSourceConnections = 0;
  private String  dataSourceDbInstance = "";
  private String  dataSourceDbUserName = "";
  private String  dataSourceDbUserPassword = "";
  private boolean dataSourceWatchdog = false;
  private String  dataSourceJdbcDriver = "";
  private String  dataSourceJdbcSource = "";

  private String  mciProfLocation = "";
  private String  mciHost = "";
  private int     mciPort = 0;
  private String  mciUsercode = "";
  private String  mciUserpassword = "";
  private String  mciNvtIODevice = "";
  private String  mciSmeAddresses = "";
  private String  voiceMailAddresses = "";

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
        daysValid  = getConfig().getInt("MCISme.DaysValid");
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
        try { defaultInform = getConfig().getBool("MCISme.defaultInform"); }  catch (Throwable th) {
          defaultInform = true;
          logger.warn("Parameter 'MCISme.defaultInform' wasn't specified. Defaul profile inform flag is on");
        }
        try { defaultNotify = getConfig().getBool("MCISme.defaultNotify"); }  catch (Throwable th) {
          defaultNotify = false;
          logger.warn("Parameter 'MCISme.defaultNotify' wasn't specified. Defaul profile notify flag is off");
        }
        int defaultReasonsMask;
        try { defaultReasonsMask = getConfig().getInt("MCISme.defaultReasonsMask"); } catch (Throwable th) {
          defaultReasonsMask = MASK_ALL;
          logger.warn("Parameter 'MCISme.defaultReasonsMask' wasn't specified. Using full profile reasons mask");
        }
        setDefaultReasonsMask(defaultReasonsMask);

        int maxCallersCount = -1;
        try { maxCallersCount = getConfig().getInt("MCISme.maxCallersCount"); } catch (Throwable th) {
          maxCallersCount = -1;
          logger.warn("Parameter 'MCISme.maxCallersCount' wasn't specified. Callers check disabled");
        }
        int maxMessagesCount = -1;
        try { maxMessagesCount = getConfig().getInt("MCISme.maxMessagesCount"); } catch (Throwable th) {
          maxMessagesCount = -1;
          logger.warn("Parameter 'MCISme.maxMessagesCount' wasn't specified. Messages check disabled");
        }
        if (maxCallersCount >= 0 && maxMessagesCount >= 0)
          throw new Exception("Only one constraint could be defined. Either 'callers', or 'messages'");
        if (maxCallersCount < 0 && maxMessagesCount < 0) {
          constraintValue = -1; constraintType = NO_CONSTRAINT;
        } else if (maxMessagesCount >= 0) {
          constraintValue = maxMessagesCount; constraintType = MAX_MESSAGES_CONSTRAINT;
        } else if (maxCallersCount >= 0) {
          constraintValue = maxCallersCount; constraintType = MAX_CALLERS_CONSTRAINT;
        }

        try {
          mciProfLocation = getConfig().getString(MCI_PROF_LOCATION_PARAM);
          if (mciProfLocation != null && mciProfLocation.length() > 0) {
            mciHost = getConfig().getString("MCISme.MSC.host");
            mciPort = getConfig().getInt   ("MCISme.MSC.port");
            mciNvtIODevice  = getConfig().getString("MCISme.MSC.nvtIODevice");
            mciUsercode     = getConfig().getString("MCISme.MSC.usercode");
            mciUserpassword = getConfig().getString("MCISme.MSC.userpassword");
            mciSmeAddresses    = getConfig().getString("MCISme.MSC.mcisme");
            voiceMailAddresses = getConfig().getString("MCISme.MSC.voicemail");
          }
        } catch (Throwable th) {
          logger.warn("Parameter '"+MCI_PROF_LOCATION_PARAM+"' wasn't specified");
        }
        if (mciSmeAddresses == null ||
            mciSmeAddresses.trim().length() <= 0) mciSmeAddresses = address;

        smppThreadPoolMax = getConfig().getInt("MCISme.SMPPThreadPool.max");
        smppThreadPoolInit = getConfig().getInt("MCISme.SMPPThreadPool.init");

        adminHost = getConfig().getString("MCISme.Admin.host");
        adminPort = getConfig().getInt("MCISme.Admin.port");

        smscHost = getConfig().getString("MCISme.SMSC.host");
        smscPort = getConfig().getInt("MCISme.SMSC.port");
        smscSid = getConfig().getString("MCISme.SMSC.sid");
        smscTimeout = getConfig().getInt("MCISme.SMSC.timeout");
        smscPassword = getConfig().getString("MCISme.SMSC.password");

        try { releaseStrategy = getConfig().getInt("MCISme.Reasons.strategy"); } catch (Throwable th) {
          logger.warn("Parameter 'MCISme.Reasons.strategy' wasn't specified, using default (redirect)");
          releaseStrategy = RELEASE_REDIRECT_STRATEGY;
        }
        try { skipUnknownCaller = getConfig().getBool("MCISme.Reasons.skipUnknownCaller"); } catch (Throwable th) {
          logger.warn("Parameter 'MCISme.Reasons.skipUnknownCaller' wasn't specified, using default (false)");
          skipUnknownCaller = false;
        }
        try { causeDetach = getConfig().getInt("MCISme.Reasons.Detach.cause"); } catch (Throwable th) {
          logger.warn("Parameter 'MCISme.Reasons.Detach.cause' wasn't specified, using default (20)");
          causeDetach = 20;
        }
        try { informDetach = getConfig().getBool("MCISme.Reasons.Detach.inform"); } catch (Throwable th) {
          logger.warn("Parameter 'MCISme.Reasons.Detach.inform' wasn't specified, using default (false)");
          informDetach = false;
        }
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

        try { redirectionAddress = getConfig().getString("MCISme.redirectionAddress"); } catch (Throwable th) {
          redirectionAddress = address;
        }
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

  private void setDefaultReasonsMask(int defaultReasonsMask) {
    defaultBusy          = ((defaultReasonsMask & MASK_BUSY) == MASK_BUSY);
    defaultAbsent        = ((defaultReasonsMask & MASK_ABSENT) == MASK_ABSENT);
    defaultDetach        = ((defaultReasonsMask & MASK_DETACH) == MASK_DETACH);
    defaultNoReply       = ((defaultReasonsMask & MASK_NOREPLY) == MASK_NOREPLY);
    defaultUnconditional = ((defaultReasonsMask & MASK_UNCOND) == MASK_UNCOND);
  }
  private int getDefaultReasonsMask() {
    int mask = MASK_NONE;
    if (defaultBusy)          mask |= MASK_BUSY;
    if (defaultAbsent)        mask |= MASK_ABSENT;
    if (defaultDetach)        mask |= MASK_DETACH;
    if (defaultNoReply)       mask |= MASK_NOREPLY;
    if (defaultUnconditional) mask |= MASK_UNCOND;
    return mask;
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
    getConfig().setString("MCISme.redirectionAddress", redirectionAddress);
    getConfig().setString("MCISme.SvcType", svcType);
    getConfig().setInt   ("MCISme.ProtocolId",  protocolId);
    getConfig().setInt   ("MCISme.DaysValid", daysValid);
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
    getConfig().setBool  ("MCISme.defaultInform", defaultInform);
    getConfig().setBool  ("MCISme.defaultNotify", defaultNotify);
    getConfig().setInt   ("MCISme.defaultReasonsMask", getDefaultReasonsMask());

    if (constraintType == NO_CONSTRAINT) {
      getConfig().setInt("MCISme.maxCallersCount", -1);
      getConfig().setInt("MCISme.maxMessagesCount", -1);
    } else if (constraintType == MAX_MESSAGES_CONSTRAINT) {
      getConfig().setInt("MCISme.maxCallersCount", -1);
      getConfig().setInt("MCISme.maxMessagesCount", constraintValue);
    } else if (constraintType == MAX_CALLERS_CONSTRAINT) {
      getConfig().setInt("MCISme.maxCallersCount", constraintValue);
      getConfig().setInt("MCISme.maxMessagesCount", -1);
    }

    getConfig().setString(MCI_PROF_LOCATION_PARAM, mciProfLocation);
    if (mciProfLocation != null && mciProfLocation.trim().length() > 0) {
      getConfig().setString("MCISme.MSC.host", mciHost);
      getConfig().setInt   ("MCISme.MSC.port", mciPort);
      getConfig().setString("MCISme.MSC.usercode", mciUsercode);
      getConfig().setString("MCISme.MSC.userpassword", mciUserpassword);
      getConfig().setString("MCISme.MSC.nvtIODevice", mciNvtIODevice);
      getConfig().setString("MCISme.MSC.mcisme", mciSmeAddresses);
      getConfig().setString("MCISme.MSC.voicemail", voiceMailAddresses);
    }

    getConfig().setInt   ("MCISme.SMPPThreadPool.max", smppThreadPoolMax);
    getConfig().setInt   ("MCISme.SMPPThreadPool.init", smppThreadPoolInit);

    getConfig().setString("MCISme.Admin.host", adminHost);
    getConfig().setInt   ("MCISme.Admin.port", adminPort);

    getConfig().setString("MCISme.SMSC.host", smscHost);
    getConfig().setInt   ("MCISme.SMSC.port", smscPort);
    getConfig().setString("MCISme.SMSC.sid", smscSid);
    getConfig().setInt   ("MCISme.SMSC.timeout", smscTimeout);
    getConfig().setString("MCISme.SMSC.password", smscPassword);

    getConfig().setInt   ("MCISme.Reasons.strategy", releaseStrategy);
    getConfig().setBool  ("MCISme.Reasons.skipUnknownCaller", skipUnknownCaller);
    getConfig().setInt   ("MCISme.Reasons.Busy.cause", causeBusy);
    getConfig().setBool  ("MCISme.Reasons.Busy.inform", informBusy);
    getConfig().setInt   ("MCISme.Reasons.NoReply.cause", causeNoReply);
    getConfig().setBool  ("MCISme.Reasons.NoReply.inform", informNoReply);
    getConfig().setInt   ("MCISme.Reasons.Unconditional.cause", causeUnconditional);
    getConfig().setBool  ("MCISme.Reasons.Unconditional.inform", informUnconditional);
    getConfig().setInt   ("MCISme.Reasons.Absent.cause", causeAbsent);
    getConfig().setBool  ("MCISme.Reasons.Absent.inform", informAbsent);
    getConfig().setInt   ("MCISme.Reasons.Detach.cause", causeDetach);
    getConfig().setBool  ("MCISme.Reasons.Detach.inform", informDetach);
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

  public String getMciProfLocation() {
    return mciProfLocation;
  }
  public void setMciProfLocation(String mciProfLocation) {
    this.mciProfLocation = mciProfLocation;
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

  public String getRedirectionAddress() {
    return redirectionAddress;
  }
  public void setRedirectionAddress(String redirectionAddress) {
    this.redirectionAddress = redirectionAddress;
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
  public boolean isDefaultInform() {
    return defaultInform;
  }
  public void setDefaultInform(boolean defaultInform) {
    this.defaultInform = defaultInform;
  }
  public boolean isDefaultNotify() {
    return defaultNotify;
  }
  public void setDefaultNotify(boolean defaultNotify) {
    this.defaultNotify = defaultNotify;
  }

  public boolean isDefaultBusy() {
    return defaultBusy;
  }
  public void setDefaultBusy(boolean defaultBusy) {
    this.defaultBusy = defaultBusy;
  }
  public boolean isDefaultNoReply() {
    return defaultNoReply;
  }
  public void setDefaultNoReply(boolean defaultNoReply) {
    this.defaultNoReply = defaultNoReply;
  }
  public boolean isDefaultUnconditional() {
    return defaultUnconditional;
  }
  public void setDefaultUnconditional(boolean defaultUnconditional) {
    this.defaultUnconditional = defaultUnconditional;
  }
  public boolean isDefaultAbsent() {
    return defaultAbsent;
  }
  public void setDefaultAbsent(boolean defaultAbsent) {
    this.defaultAbsent = defaultAbsent;
  }
  public boolean isDefaultDetach() {
    return defaultDetach;
  }
  public void setDefaultDetach(boolean defaultDetach) {
    this.defaultDetach = defaultDetach;
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

  public int getReleaseStrategyInt() {
    return releaseStrategy;
  }
  public String getReleaseStrategy() {
    return String.valueOf(releaseStrategy);
  }
  public void setReleaseStrategyInt(int releaseStrategy) {
    this.releaseStrategy = releaseStrategy;
  }
  public void setReleaseStrategy(String releaseStrategy) {
    try { this.releaseStrategy = Integer.decode(releaseStrategy).intValue(); }
    catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Reasons.strategy parameter value: \"" + releaseStrategy + '"', e);
    }
  }

  public boolean isSkipUnknownCaller() {
    return skipUnknownCaller;
  }
  public void setSkipUnknownCaller(boolean skipUnknownCaller) {
    this.skipUnknownCaller = skipUnknownCaller;
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

  public int getCauseDetachInt() {
    return causeDetach;
  }
  public void setCauseDetachInt(int causeDetach) {
    this.causeDetach = causeDetach;
  }
  public String getCauseDetach() {
    return String.valueOf(causeDetach);
  }
  public void setCauseDetach(String causeDetach) {
    try { this.causeDetach = Integer.decode(causeDetach).intValue(); } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.Reasons.Detach.cause parameter value: \"" + causeDetach + '"', e);
    }
  }
  public boolean isInformDetach() {
    return informDetach;
  }
  public void setInformDetach(boolean informDetach) {
    this.informDetach = informDetach;
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

  public String getMciHost() {
    return mciHost;
  }
  public void setMciHost(String mciHost) {
    this.mciHost = mciHost;
  }
  public int getMciPortInt() {
    return mciPort;
  }
  public String getMciPort() {
    return String.valueOf(mciPort);
  }
  public void setMciPortInt(int mciPort) {
    this.mciPort = mciPort;
  }
  public void setMciPort(String mciPort) {
    try { this.mciPort = Integer.decode(mciPort).intValue(); } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.MSC.port parameter value: \"" + mciPort + '"', e);
    }
  }
  public String getMciNvtIODevice() {
    return mciNvtIODevice;
  }
  public void setMciNvtIODevice(String mciNvtIODevice) {
    this.mciNvtIODevice = mciNvtIODevice;
  }
  public String getMciUsercode() {
    return mciUsercode;
  }
  public void setMciUsercode(String mciUsercode) {
    this.mciUsercode = mciUsercode;
  }
  public String getMciUserpassword() {
    return mciUserpassword;
  }
  public void setMciUserpassword(String mciUserpassword) {
    this.mciUserpassword = mciUserpassword;
  }
  public String getVoiceMailAddresses() {
    return voiceMailAddresses;
  }
  public void setVoiceMailAddresses(String voiceMailAddresses) {
    this.voiceMailAddresses = voiceMailAddresses;
  }
  public String getMciSmeAddresses() {
    return mciSmeAddresses;
  }
  public void setMciSmeAddresses(String mciSmeAddresses) {
    this.mciSmeAddresses = mciSmeAddresses;
  }

  public int getConstraintValueInt() {
    return constraintValue;
  }
  public void setConstraintValueInt(int value) {
    this.constraintValue = value;
  }
  public String getConstraintValue() {
    return (constraintValue < 0) ? "":String.valueOf(constraintValue);
  }
  public void setConstraintValue(String constraintValue)
  {
    if (constraintValue == null || constraintValue.trim().length() <= 0) this.constraintValue = -1;
    else {
      try {
        this.constraintValue = Integer.decode(constraintValue).intValue();
      } catch (NumberFormatException e) {
        logger.debug("Invalid int constraint value: " + constraintValue + '"', e);
      }
    }
  }

  public int getConstraintType() {
    return constraintType;
  }
  public void setConstraintType(int constraintType) {
    this.constraintType = constraintType;
  }

}
