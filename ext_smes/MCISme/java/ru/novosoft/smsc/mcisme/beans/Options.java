package ru.novosoft.smsc.mcisme.beans;

import ru.novosoft.smsc.util.SortedList;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 18:37:43
 * To change this template use Options | File Templates.
 */
public class Options extends MCISmeBean
{
  public static final String SCHEDTABLE_SECTION_NAME = "MCISme.Scheduling.schedule";
  private String address = "";
  private String redirectionAddress = "";
  private String svcType = "";
  private String callingMask = "";
  private String calledMask = "";
  private int protocolId = 0;
  private int daysValid = 1;
  private int maxDataSmRegistrySize = 0;
  private int pduDispatchersCount = 4;
  private int unrespondedMessagesMax = 0;
  private int unrespondedMessagesSleep = 0;
  private int outgoingSpeedMax = 0;
  private String responceWaitTime = "";
  private int inputQueueSize=0;
  private int maxRowsPerMessage=0;
  private boolean forceInform = false;
  private boolean forceNotify = false;
  private boolean defaultInform = false;
  private boolean defaultNotify = false;
  private boolean defaultWantNotifyMe = false;
  private boolean useWantNotifyPolicy = false;
  private boolean groupSmsByCallingAbonent = false;


/*  public final static int NO_CONSTRAINT           = 0;
  public final static int MAX_CALLERS_CONSTRAINT  = 10;
  public final static int MAX_MESSAGES_CONSTRAINT = 20;
  private int constraintType  = NO_CONSTRAINT;
  private int constraintValue = -1;*/

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

//  private String  dataSourceType = "";
//  private int     dataSourceConnections = 0;
//  private String  dataSourceDbInstance = "";
//  private String  dataSourceDbUserName = "";
//  private String  dataSourceDbUserPassword = "";
//  private boolean dataSourceWatchdog = false;
//  private String  dataSourceJdbcDriver = "";
//  private String  dataSourceJdbcSource = "";

  private  String newError = null;
  private  String newErrorTimeout = null;

  private String  mciProfLocation = "";
  private String  mciHost = "";
  private int     mciPort = 0;
  private String  mciUsercode = "";
  private String  mciUserpassword = "";
  private String  mciNvtIODevice = "";
  private String  mciSmeAddresses = "";
  private String  voiceMailAddresses = "";

  private String  countryCode = "";
  private String  timeZoneFileLocation = "";
  private String  routesFileLocation = "";

  private String    eventStorageLocation = "";
  private String    eventLifeTime = "";
  private String    eventPolicyRegistration = "";
  private int       maxEvents = 0;
  private int       bdFilesIncr = 0;

  private String    profStorageLocation = "";
  private String    profStorageHost = "";
  private int       profStoragePort = 0;
  private String    statDir = "";

  private String   resendingPeriod = "";
  private String   schedOnBusy = "";

  private boolean initialized = false;

  private String mbDone = null;
  private String mbCancel = null;

  private String    advertServer = "";
  private int       advertPort = 0;
  private int       advertTimeout = 0;
  private boolean	useAdvert = false;

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
        maxDataSmRegistrySize = getConfig().getInt("MCISme.maxDataSmRegistrySize");
        pduDispatchersCount = getConfig().getInt("MCISme.pduDispatchersCount");
        unrespondedMessagesMax = getConfig().getInt("MCISme.unrespondedMessagesMax");
        unrespondedMessagesSleep = getConfig().getInt("MCISme.unrespondedMessagesSleep");
        outgoingSpeedMax = getConfig().getInt("MCISme.outgoingSpeedMax");
        responceWaitTime = getConfig().getString("MCISme.responceWaitTime");
        inputQueueSize = getConfig().getInt("MCISme.inputQueueSize");
        maxRowsPerMessage = getConfig().getInt("MCISme.maxRowsPerMessage");
        forceInform = getConfig().getBool("MCISme.forceInform");
        forceNotify = getConfig().getBool("MCISme.forceNotify");
        try { defaultInform = getConfig().getBool("MCISme.defaultInform"); }  catch (Throwable th) {
          defaultInform = true;
          logger.warn("Parameter 'MCISme.defaultInform' wasn't specified. Default profile inform flag is on");
        }
        try { defaultNotify = getConfig().getBool("MCISme.defaultNotify"); }  catch (Throwable th) {
          defaultNotify = false;
          logger.warn("Parameter 'MCISme.defaultNotify' wasn't specified. Default profile notify flag is off");
        }
        try { defaultWantNotifyMe = getConfig().getBool("MCISme.defaultWantNotifyMe"); }  catch (Throwable th) {
          defaultWantNotifyMe = false;
          logger.warn("Parameter 'MCISme.defaultWantNotifyMe' wasn't specified. Default profile want notify flag is off");
        }
        int defaultReasonsMask;
        try { defaultReasonsMask = getConfig().getInt("MCISme.defaultReasonsMask"); } catch (Throwable th) {
          defaultReasonsMask = MASK_ALL;
          logger.warn("Parameter 'MCISme.defaultReasonsMask' wasn't specified. Using full profile reasons mask");
        }
        setDefaultReasonsMask(defaultReasonsMask);

        try { useWantNotifyPolicy = getConfig().getBool("MCISme.useWantNotifyPolicy"); }  catch (Throwable th) {
          useWantNotifyPolicy = false;
          logger.warn("Parameter 'MCISme.useWantNotifyPolicy' wasn't specified. Want notify policy is off");
        }

        try { groupSmsByCallingAbonent = getConfig().getBool("MCISme.GroupSmsByCallingAbonent"); }  catch (Throwable th) {
          groupSmsByCallingAbonent = false;
          logger.warn("Parameter 'MCISme.GroupSmsByCallingAbonent' wasn't specified. Want notify policy is off");
        }
/*        int maxCallersCount;
        try { maxCallersCount = getConfig().getInt("MCISme.maxCallersCount"); } catch (Throwable th) {
          maxCallersCount = -1;
          logger.warn("Parameter 'MCISme.maxCallersCount' wasn't specified. Callers check disabled");
        }
        int maxMessagesCount;
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
*/
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

		countryCode = getConfig().getString("MCISme.CountryCode");
		timeZoneFileLocation = getConfig().getString("MCISme.timeZoneFileLocation");
		routesFileLocation = getConfig().getString("MCISme.routesFileLocation");

		eventStorageLocation = getConfig().getString("MCISme.Storage.location");
        eventLifeTime = getConfig().getString("MCISme.Storage.eventLifeTime");
        eventPolicyRegistration = getConfig().getString("MCISme.Storage.eventRegistrationPolicy");
        maxEvents = getConfig().getInt("MCISme.Storage.maxEvents");
        bdFilesIncr = getConfig().getInt("MCISme.Storage.bdFilesIncr");

  	    advertServer = getConfig().getString("MCISme.Advertising.server");
		advertPort = getConfig().getInt("MCISme.Advertising.port");
		advertTimeout = getConfig().getInt("MCISme.Advertising.timeout");
		useAdvert = getConfig().getBool("MCISme.Advertising.useAdvert");

        profStorageLocation = getConfig().getString("MCISme.ProfileStorage.location");
        profStoragePort = getConfig().getInt("MCISme.ProfileStorage.port");
        profStorageHost = getConfig().getString("MCISme.ProfileStorage.host");

        resendingPeriod = getConfig().getString("MCISme.Scheduling.resendingPeriod");
        schedOnBusy = getConfig().getString("MCISme.Scheduling.schedOnBusy");

        statDir = getConfig().getString("MCISme.Statistics.statDir");

      } catch (Exception e) {
        logger.error(e);
        return error("mcisme.error.config_load_failed", e);
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

    if (mbDone != null)   return save(request.getParameterMap());
    if (mbCancel != null) return RESULT_DONE;

    return result;
  }

  private int save(Map requestParams)
  {
    getConfig().setString("MCISme.Address", address);
    getConfig().setString("MCISme.redirectionAddress", redirectionAddress);
    getConfig().setString("MCISme.SvcType", svcType);
    getConfig().setInt   ("MCISme.ProtocolId",  protocolId);
    getConfig().setInt   ("MCISme.DaysValid", daysValid);
    getConfig().setString("MCISme.CallingMask", callingMask);
    getConfig().setString("MCISme.CalledMask" , calledMask);
    getConfig().setInt   ("MCISme.pduDispatchersCount", pduDispatchersCount);
    getConfig().setInt   ("MCISme.maxDataSmRegistrySize", maxDataSmRegistrySize);
    getConfig().setInt   ("MCISme.unrespondedMessagesMax", unrespondedMessagesMax);
    getConfig().setInt   ("MCISme.unrespondedMessagesSleep", unrespondedMessagesSleep);
    getConfig().setInt   ("MCISme.outgoingSpeedMax", outgoingSpeedMax);
    getConfig().setString("MCISme.responceWaitTime", responceWaitTime);
    getConfig().setInt   ("MCISme.inputQueueSize", inputQueueSize);
    getConfig().setInt   ("MCISme.maxRowsPerMessage", maxRowsPerMessage);
    getConfig().setBool  ("MCISme.forceInform", forceInform);
    getConfig().setBool  ("MCISme.forceNotify", forceNotify);
    getConfig().setBool  ("MCISme.defaultInform", defaultInform);
    getConfig().setBool  ("MCISme.defaultNotify", defaultNotify);
    getConfig().setBool  ("MCISme.defaultWantNotifyMe", defaultWantNotifyMe);
    getConfig().setInt   ("MCISme.defaultReasonsMask", getDefaultReasonsMask());
    getConfig().setBool  ("MCISme.useWantNotifyPolicy", useWantNotifyPolicy);
    getConfig().setBool  ("MCISme.GroupSmsByCallingAbonent", groupSmsByCallingAbonent);

/*    if (constraintType == NO_CONSTRAINT) {
      getConfig().setInt("MCISme.maxCallersCount", -1);
      getConfig().setInt("MCISme.maxMessagesCount", -1);
    } else if (constraintType == MAX_MESSAGES_CONSTRAINT) {
      getConfig().setInt("MCISme.maxCallersCount", -1);
      getConfig().setInt("MCISme.maxMessagesCount", constraintValue);
    } else if (constraintType == MAX_CALLERS_CONSTRAINT) {
      getConfig().setInt("MCISme.maxCallersCount", constraintValue);
      getConfig().setInt("MCISme.maxMessagesCount", -1);
    }
*/
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

//    getConfig().setString("MCISme.DataSource.type", dataSourceType);
//    getConfig().setInt   ("MCISme.DataSource.connections", dataSourceConnections);
//    getConfig().setString("MCISme.DataSource.dbInstance", dataSourceDbInstance);
//    getConfig().setString("MCISme.DataSource.dbUserName", dataSourceDbUserName);
//    getConfig().setString("MCISme.DataSource.dbUserPassword", dataSourceDbUserPassword);
//    getConfig().setBool  ("MCISme.DataSource.watchdog", dataSourceWatchdog);
//    getConfig().setString("MCISme.DataSource.jdbc.driver", dataSourceJdbcDriver);
//    getConfig().setString("MCISme.DataSource.jdbc.source", dataSourceJdbcSource);

	getConfig().setString("MCISme.CountryCode", countryCode);
	getConfig().setString("MCISme.timeZoneFileLocation", timeZoneFileLocation);
	getConfig().setString("MCISme.routesFileLocation", routesFileLocation);

    getConfig().setString("MCISme.Storage.location", eventStorageLocation);
    getConfig().setString("MCISme.Storage.eventLifeTime", eventLifeTime);
    getConfig().setString("MCISme.Storage.eventRegistrationPolicy", eventPolicyRegistration);
    getConfig().setInt("MCISme.Storage.maxEvents", maxEvents);
    getConfig().setInt("MCISme.Storage.bdFilesIncr", bdFilesIncr);

	getConfig().setString("MCISme.Advertising.server", advertServer);
	getConfig().setInt("MCISme.Advertising.port", advertPort);
	getConfig().setInt("MCISme.Advertising.timeout", advertTimeout);
	getConfig().setBool("MCISme.Advertising.useAdvert", useAdvert);

    getConfig().setString("MCISme.ProfileStorage.location", profStorageLocation);
    getConfig().setInt("MCISme.ProfileStorage.port", profStoragePort);
    getConfig().setString("MCISme.ProfileStorage.host", profStorageHost);

    getConfig().setString("MCISme.Scheduling.resendingPeriod", resendingPeriod);
    getConfig().setString("MCISme.Scheduling.schedOnBusy", schedOnBusy);

    getConfig().setString("MCISme.Statistics.statDir", statDir);

    final String PREFIX = "err.to.";
    getConfig().removeSection(SCHEDTABLE_SECTION_NAME);
    for (Iterator i = requestParams.keySet().iterator(); i.hasNext();)
    {
      String paramName = (String) i.next();
      if (paramName.startsWith(PREFIX))
      {
        final String paramValue = getParamValue(requestParams.get(paramName));
        if (paramValue != null)
            getConfig().setString(SCHEDTABLE_SECTION_NAME + '.' + paramValue, getParamValue(requestParams.get(paramName.substring(4))));
      }
    }
    getMCISmeContext().setChangedOptions(true);
    return RESULT_DONE;
  }

  private String getParamValue(Object paramObjectValue)
  {
    if (paramObjectValue instanceof String)
      return (String) paramObjectValue;
    else if (paramObjectValue instanceof String[]) {
      String[] paramValues = (String[]) paramObjectValue;
      StringBuffer result = new StringBuffer();
      for (int i = 0; i < paramValues.length; i++) {
        result.append(paramValues[i]);
      }
      return result.toString();
    } else
      return null;
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

  public int getMaxDataSmRegistrySizeInt() {
    return maxDataSmRegistrySize;
  }
  public void setMaxDataSmRegistrySizeInt(int maxDataSmRegistrySize) {
    this.maxDataSmRegistrySize = maxDataSmRegistrySize;
  }

  public String getMaxDataSmRegistrySize() {
    return String.valueOf(maxDataSmRegistrySize);
  }
  public void setMaxDataSmRegistrySize(String maxDataSmRegistrySize) {
    try {
      this.maxDataSmRegistrySize = Integer.decode(maxDataSmRegistrySize).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.maxDataSmRegistrySize parameter value: " + maxDataSmRegistrySize + '"', e);
    }
  }

  public int getPduDispatchersCountInt() {
    return pduDispatchersCount;
  }
  public void setPduDispatchersCountInt(int pduDispatchersCount) {
    this.pduDispatchersCount = pduDispatchersCount;
  }

  public String getPduDispatchersCount() {
    return String.valueOf(pduDispatchersCount);
  }
  public void setPduDispatchersCount(String pduDispatchersCount) {
    try {
      this.pduDispatchersCount = Integer.decode(pduDispatchersCount).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.pduDispatchersCount parameter value: " + pduDispatchersCount + '"', e);
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

  public int getOutgoingSpeedMaxInt() {
    return outgoingSpeedMax;
  }
  public void setOutgoingSpeedMaxInt(int value) {
    this.outgoingSpeedMax = value;
  }
  public String getOutgoingSpeedMax() {
    return String.valueOf(outgoingSpeedMax);
  }
  public void setOutgoingSpeedMax(String value)
  {
    try {
      this.outgoingSpeedMax = Integer.decode(value).intValue();
    } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.outgoingSpeedMax parameter value: " + outgoingSpeedMax + '"', e);
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

  public boolean isDefaultWantNotifyMe() {
    return defaultWantNotifyMe;
  }

  public void setDefaultWantNotifyMe(boolean defaultWantNotifyMe) {
    this.defaultWantNotifyMe = defaultWantNotifyMe;
  }

  public boolean isUseWantNotifyPolicy() {
    return useWantNotifyPolicy;
  }

  public void setUseWantNotifyPolicy(boolean useWantNotifyPolicy) {
    this.useWantNotifyPolicy = useWantNotifyPolicy;
  }

  public boolean isGroupSmsByCallingAbonent() {
    return groupSmsByCallingAbonent;
  }

  public void setGroupSmsByCallingAbonent(boolean groupSmsByCallingAbonent) {
    this.groupSmsByCallingAbonent = groupSmsByCallingAbonent;
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

/*  public String getDataSourceType() {
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
  */
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

/*  public int getConstraintValueInt() {
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
*/
	public String getCountryCode() 
	{
		return countryCode;
	}
	public void setCountryCode(String value) 
	{
		this.countryCode = value;
	}
	public String getTimeZoneFileLocation() 
	{
		return timeZoneFileLocation;
	}
	public void setTimeZoneFileLocation(String value) 
	{
		this.timeZoneFileLocation = value;
	}
	public String getRoutesFileLocation() 
	{
		return routesFileLocation;
	}
	public void setRoutesFileLocation(String value) 
	{
		this.routesFileLocation = value;
	}

    public String getEventStorageLocation() 
	{
      return eventStorageLocation;
    }
    public void setEventStorageLocation(String value) {
      this.eventStorageLocation = value;
    }
    public String getEventLifeTime() {
      return eventLifeTime;
    }
    public void setEventLifeTime(String value) {
      this.eventLifeTime = value;
    }
    public String getEventPolicyRegistration() {
      return eventPolicyRegistration;
    }
    public void setEventPolicyRegistration(String value) {
      this.eventPolicyRegistration = value;
    }
    public int getMaxEventsInt() {
      return maxEvents;
    }
    public String getMaxEvents() {
      return String.valueOf(maxEvents);
    }

    public void setMaxEventsInt(int value) {
      this.maxEvents = value;
    }

    public void setMaxEvents(String value) {
      try { this.maxEvents = Integer.decode(value).intValue(); } catch (NumberFormatException e) {
        logger.debug("Invalid int MCISme.MSC.port parameter value: \"" + value + '"', e);
      }
    }

    public int getBdFilesIncrInt() {
      return bdFilesIncr;
    }
    public String getBdFilesIncr() {
      return String.valueOf(bdFilesIncr);
    }

    public void setBdFilesIncrInt(int value) {
      this.bdFilesIncr = value;
    }

    public void setBdFilesIncr(String value) {
      try { this.bdFilesIncr = Integer.decode(value).intValue(); } catch (NumberFormatException e) {
        logger.debug("Invalid int MCISme.MSC.port parameter value: \"" + value + '"', e);
      }
    }

	
	public String getAdvertServer() 
	{
		return advertServer;
	}
	public void setAdvertServer(String value) 
	{
		this.advertServer = value;
	}

	public String getAdvertPort() 
	{
		return String.valueOf(advertPort);
	}
	public void setAdvertPortInt(int value) 
	{
		this.advertPort = value;
	}
	public void setAdvertPort(String value) 
	{
		try { this.advertPort = Integer.decode(value).intValue(); } 
		catch (NumberFormatException e) 
		{
			logger.debug("Invalid int MCISme.Advertising.port parameter value: \"" + value + '"', e);
		}
	}

	public String getAdvertTimeout() 
	{
		return String.valueOf(advertTimeout);
	}
	public void setAdvertTimeoutInt(int value) 
	{
		this.advertTimeout = value;
	}
	public void setAdvertTimeout(String value) 
	{
		try { this.advertTimeout = Integer.decode(value).intValue(); } 
		catch (NumberFormatException e) 
		{
			logger.debug("Invalid int MCISme.Advertising.timeout parameter value: \"" + value + '"', e);
		}
	}
	
	public boolean isUseAdvert() 
	{
		return useAdvert;
	}
	public void setUseAdvert(boolean value) 
	{
		this.useAdvert = value;
	}

  public String getProfStorageLocation() 
  {
    return profStorageLocation;
  }
  public void setProfStorageLocation(String value) {
    this.profStorageLocation = value;
  }

  public String getProfStorageHost() {
    return profStorageHost;
  }

  public void setProfStorageHost(String profStorageHost) {
    this.profStorageHost = profStorageHost;
  }

  public int getProfStoragePortInt() {
    return profStoragePort;
  }
  public String getProfStoragePort() {
    return String.valueOf(profStoragePort);
  }

  public void setProfStoragePortInt(int value) {
    this.profStoragePort = value;
  }

  public void setProfStoragePort(String value) {
    try { this.profStoragePort = Integer.decode(value).intValue(); } catch (NumberFormatException e) {
      logger.debug("Invalid int MCISme.MSC.port parameter value: \"" + value + '"', e);
    }
  }

    public String getResendingPeriod() {
      return resendingPeriod;
    }
    public void setResendingPeriod(String value) {
      this.resendingPeriod = value;
    }
    public String getSchedOnBusy() {
      return schedOnBusy;
    }
    public void setSchedOnBusy(String value) {
      this.schedOnBusy = value;
    }

  public String getStatDir() {
    return statDir;
  }
  public void setStatDir(String value) {
    this.statDir = value;
  }

  public List getErrorNumbers() {
    return new SortedList(getConfig().getSectionChildShortParamsNames(SCHEDTABLE_SECTION_NAME));
  }
  public String getOnErrorTimeout(String errorNumber)
  {
      String result = null;
      try
      {
        result = getConfig().getString(SCHEDTABLE_SECTION_NAME + '.' + errorNumber);
      } catch (Exception e) {
        logger.error(e);
        result = "00:00:00";
      }
    return result;
  }
}
