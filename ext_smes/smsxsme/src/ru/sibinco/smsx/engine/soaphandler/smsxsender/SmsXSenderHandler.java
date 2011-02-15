package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import com.eyeline.sme.utils.worker.IterativeWorker;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.ConfigException;
import org.apache.log4j.Category;
import ru.sibinco.smsx.Context;
import ru.sibinco.smsx.engine.service.CmdStatusObserver;
import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListCheckMsisdnCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarCheckMessageStatusCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.secret.commands.SecretGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.secret.commands.SecretSendMessageCmd;
import ru.sibinco.smsx.engine.service.secret.commands.SecretBatchCmd;
import ru.sibinco.smsx.engine.service.secret.commands.SecretGetBatchStatusCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderSendMessageCmd;
import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.advertising.AdvertisingClientException;

import java.io.File;
import java.io.InputStream;
import java.rmi.RemoteException;
import java.util.Date;

class SmsXSenderHandler  {

  private static final Category log = Category.getInstance(SmsXSenderHandler.class);

  private static final String CALENDAR_MSG_ID_PREFIX = "0";
  private static final String SECRET_MSG_ID_PREFIX = "1";
  private static final String SENDER_MSG_ID_PREFIX = "2";

  // Secret service errors
  private static final int STATUS_SECRET_WRONG_DESTINATION_ADDRESS = -201;
  private static final int STATUS_SECRET_DESTINATIONS_ARE_EMPTY = -202;

  // Calendar service errors
  private static final int STATUS_CALENDAR_WRONG_SEND_DATE = -100;
  private static final int STATUS_CALENDAR_WRONG_DESTINATION_ADDRESS = -101;

  // General error
  private static final int STATUS_DESTINATION_ABONENT_IN_BLACK_LIST = -6;
  private static final int STATUS_DESTINATION_ABONENT_UNKNOWN = -5;
  private static final int STATUS_INVALID_MESSAGE_ID = -4;
  private static final int STATUS_MESSAGE_NOT_FOUND = -3;
  private static final int STATUS_SYSTEM_ERROR = -2;
  private static final int STATUS_DELIVERY_ERROR = -1;
  private static final int STATUS_ACCEPTED = 0;
  private static final int STATUS_DELIVERED = 1;

  private final String serviceAddress;
  private final int advertisingRestriction;
  private final String advertisingDelimiter;
  private final String advertisingClientName;
  private final String mscAddress;
  private boolean appendAdvertising;

  private final AdvertisingClient advertisingClient;

  SmsXSenderHandler(String configDir, AdvertisingClient advertisingClient) throws SOAPHandlerInitializationException {

    this.advertisingClient = advertisingClient;

    final File configFile = new File(configDir, "soaphandlers/smsxsendhandler.properties");

    try {
      final PropertiesConfig config = new PropertiesConfig();
      config.load(configFile);
      serviceAddress = config.getString("service.address");
      appendAdvertising = config.getBool("append.advertising");
      advertisingClientName = config.getString("advertising.service.name");
      advertisingRestriction = config.getInt("advertising.restriction");
      advertisingDelimiter = config.getString("advertising.delimiter");
      mscAddress = config.getString("msc.address");
    } catch (ConfigException e) {
      throw new SOAPHandlerInitializationException(e);
    }

    new ConfigChecker(configFile, 60000).start();
  }

  public SmsXSenderResponse sendSms(String msisdn, String message, boolean express, boolean secret, boolean calendar, long calendarTimeUTC, boolean advertising) throws RemoteException {
    final long start = System.currentTimeMillis();
    try {
      if (log.isInfoEnabled())
        log.info("Send SMS: dstaddr=" + msisdn + "; msg=" + message + "; express=" + express + "; secret=" + secret + "; calendar=" + calendar + "; time=" + new Date(calendarTimeUTC) + "; advertising=" + advertising);

      // Check operator
      final String operator = Context.getInstance().getOperators().getOperatorByAddress(msisdn);
      if (operator == null) {
        log.error("Unknown or incorrect operator for dstaddr=" + msisdn);
        return new SmsXSenderResponse(null, -1, STATUS_DESTINATION_ABONENT_UNKNOWN);
      }

      // Check black list
      final BlackListCheckMsisdnCmd cmd = new BlackListCheckMsisdnCmd();
      cmd.setMsisdn(msisdn);
      boolean inBlackList = Services.getInstance().getBlackListService().execute(cmd);

      if (inBlackList) {
        log.error("Dstaddr=" + msisdn + " is in black list");
        return new SmsXSenderResponse(null, -1, STATUS_DESTINATION_ABONENT_IN_BLACK_LIST);
      }

      if (calendar)
        return sendCalendarMessage(serviceAddress, msisdn, message, express, calendarTimeUTC, null, advertising);
      else if (secret)
        return sendSecretMessage(serviceAddress, msisdn, message, express, null, advertising);
      else
        return sendSenderMessage(serviceAddress, msisdn, message, express, null, advertising);
      
    } catch (Throwable e) {
      log.error(e,e);
      return new SmsXSenderResponse(null, -1, STATUS_SYSTEM_ERROR);

    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public SmsXSenderResponse sendPaidSms(String oa, String da, String message, boolean express, boolean secret, boolean calendar, long calendarTimeUTC) throws RemoteException {
    final long start = System.currentTimeMillis();
    try {
      if (log.isInfoEnabled())
        log.info("Send SMS: oa=" + oa + "; da=" + da + "; msg=" + message + "; express=" + express + "; secret=" + secret + "; calendar=" + calendar + "; time=" + new Date(calendarTimeUTC));

      if (calendar)
        return sendCalendarMessage(oa, da, message, express, calendarTimeUTC, mscAddress, false);
      else if (secret)
        return sendSecretMessage(oa, da, message, express, mscAddress, false);
      else
        return sendSenderMessage(oa, da, message, express, mscAddress, false);

    } catch (Throwable e) {
      log.error(e,e);
      return new SmsXSenderResponse(null, -1, STATUS_SYSTEM_ERROR);

    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public SmsXSenderResponse sendSysSms(String oa, String da, String message) throws RemoteException {
    final long start = System.currentTimeMillis();
    try {
      if (log.isInfoEnabled())
        log.info("Send System SMS: oa=" + oa + "; da=" + da + "; msg=" + message);

      SenderSendMessageCmd c = new SenderSendMessageCmd();
      c.setSourceAddress(oa);
      c.setDestinationAddress(da);
      c.setMessage(message);
      c.setDestAddressSubunit(-1);
      c.setStorable(true);
      c.setSystemMessage(true);
      c.setSourceId(AsyncCommand.SOURCE_SOAP);

      int status;
      String id_message=null;
      final CmdStatusObserver observer = new CmdStatusObserver(null);
      c.addExecutionObserver(observer);
      Services.getInstance().getSenderService().execute(c);
      observer.waitStatus();
      switch (c.getStatus()) {
        case SenderSendMessageCmd.STATUS_SUCCESS:
          status = STATUS_ACCEPTED;
          id_message = SENDER_MSG_ID_PREFIX + c.getMsgId();
          break;
        default: status = STATUS_SYSTEM_ERROR;
      }

      return new SmsXSenderResponse(id_message, 0, status);
    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  private SmsXSenderResponse sendSenderMessage(String sourceAddress, String destinationAddress, String message, boolean express, String mscAddress, boolean advertising) {

    // Prepare message
    if (advertising && appendAdvertising) {
      final String banner = (advertisingRestriction > 0) ? getBannerForAbonent(destinationAddress, advertisingRestriction - message.length() - advertisingDelimiter.length()) : getBannerForAbonent(sourceAddress);
      if (log.isInfoEnabled())
        log.info("Append banner: " + banner);
      if (banner != null)
        message += advertisingDelimiter + banner;
    }

    SenderSendMessageCmd c = new SenderSendMessageCmd();
    c.setSourceAddress(sourceAddress);
    c.setDestinationAddress(destinationAddress);
    c.setMessage(message);
    c.setDestAddressSubunit(express ? 1 : -1);
    c.setStorable(true);
    c.setMscAddress(mscAddress);
    c.setSourceId(AsyncCommand.SOURCE_SOAP);

    String id_message = null;
    int status;
    final CmdStatusObserver observer = new CmdStatusObserver(null);
    c.addExecutionObserver(observer);
    Services.getInstance().getSenderService().execute(c);
    observer.waitStatus();
    switch (c.getStatus()) {
      case SenderSendMessageCmd.STATUS_SUCCESS:
        status = STATUS_ACCEPTED;
        id_message = SENDER_MSG_ID_PREFIX + c.getMsgId();
        break;
      default:
        status = STATUS_SYSTEM_ERROR;
    }

    return new SmsXSenderResponse(id_message, 0, status);
  }

  private SmsXSenderResponse sendCalendarMessage(String sourceAddress, String destinationAddress, String message, boolean express, long calendarTimeUTC, String mscAddress, boolean advertising) {
    final CalendarSendMessageCmd c = new CalendarSendMessageCmd();
    c.setSourceAddress(sourceAddress);
    c.setDestinationAddress(destinationAddress);
    c.setSendDate(new Date(calendarTimeUTC));
    c.setMessage(message);
    c.setDestAddressSubunit(express ? 1 : -1);
    c.setStoreDeliveryStatus(true);
    c.setSourceId(AsyncCommand.SOURCE_SOAP);
    c.setMscAddress(mscAddress);
    c.setAppendAdvertising(advertising);

    String id_message = null;
    int status;
    try {
      long msgId = Services.getInstance().getCalendarService().execute(c);
      status = STATUS_ACCEPTED;
      id_message = CALENDAR_MSG_ID_PREFIX + msgId;
    } catch (CommandExecutionException e) {
      switch (e.getErrCode()) {
        case CalendarSendMessageCmd.ERR_WRONG_SEND_DATE:
          status = STATUS_CALENDAR_WRONG_SEND_DATE;
          break;
        case CalendarSendMessageCmd.ERR_WRONG_DESTINATION_ADDRESS:
          status = STATUS_CALENDAR_WRONG_DESTINATION_ADDRESS;
          break;
        default:
          status = STATUS_SYSTEM_ERROR;
      }
    }

    return new SmsXSenderResponse(id_message, 0, status);
  }

  private SmsXSenderResponse sendSecretMessage(String sourceAddress, String destinationAddress, String message, boolean express, String mscAddress, boolean advertising) throws CommandExecutionException {
    final SecretSendMessageCmd c = new SecretSendMessageCmd();
    c.setSourceAddress(sourceAddress);
    c.setDestinationAddress(destinationAddress);
    c.setMessage(message);
    c.setDestAddressSubunit(express ? 1 : -1);
    c.setSaveDeliveryStatus(true);
    c.setNotifyOriginator(false);
    c.setSourceId(AsyncCommand.SOURCE_SOAP);
    c.setMscAddress(mscAddress);
    c.setAppendAdverising(advertising);

    String id_message = null;
    int status;
    try {
      long msgId = Services.getInstance().getSecretService().execute(c);
      status = STATUS_ACCEPTED;
      id_message = SECRET_MSG_ID_PREFIX + msgId;
    } catch (CommandExecutionException e) {
      switch(e.getErrCode()) {
        case SecretSendMessageCmd.ERR_DESTINATION_ADDRESS_IS_NOT_ALLOWED:
          status = STATUS_SECRET_WRONG_DESTINATION_ADDRESS;
          break;
        default:
          status = STATUS_SYSTEM_ERROR;
      }
    }

    return new SmsXSenderResponse(id_message, 0, status);
  }

  public SmsXSenderResponse checkStatus(String messageId) throws RemoteException {
    final long start = System.currentTimeMillis();
    if (log.isInfoEnabled())
      log.info("Check status: msgId=" + messageId);

    if (messageId == null)
      return new SmsXSenderResponse(messageId, 0, STATUS_INVALID_MESSAGE_ID);

    int messageStatus = -1;
    int smppStatus = -1;
    try {

      if (messageId.startsWith(CALENDAR_MSG_ID_PREFIX))
        return getCalendarMessageStatus(messageId);

      else if (messageId.startsWith(SECRET_MSG_ID_PREFIX))
        return getSecretMessageStatus(messageId);

      else return getSenderMessageStatus(messageId);

    } catch (Throwable e) {
      messageStatus = STATUS_SYSTEM_ERROR;
    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }

    return new SmsXSenderResponse(messageId, smppStatus, messageStatus);
  }

  public SmsXSenderResponse batchSecret(String oa, String message, boolean express, InputStream destinations) throws RemoteException {
    if (log.isInfoEnabled())
      log.info("Batch secret: oa=" + oa + "; msg=" + message);
    long start = System.currentTimeMillis();

    if (destinations == null) {
      log.warn("Destinations are empty.");
      return new SmsXSenderResponse(null, 0, STATUS_SECRET_DESTINATIONS_ARE_EMPTY);
    }

    SecretBatchCmd cmd = new SecretBatchCmd();
    cmd.setDestAddressSubunit(express ? 1 : 0);
    cmd.setDestinations(destinations);
    cmd.setMessage(message);
    cmd.setSourceAddress(oa);
    cmd.setSourceId(SecretBatchCmd.SOURCE_SOAP);

    try {
      String id = Services.getInstance().getSecretService().execute(cmd);

      return new SmsXSenderResponse(id, 0, STATUS_ACCEPTED);
    } catch (Throwable e) {
      log.error(e,e);
      return new SmsXSenderResponse(null, 0, STATUS_SYSTEM_ERROR);
    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public SmsXSenderResponse checkBatchSecretStatus(String msgId) throws RemoteException {
    if (log.isInfoEnabled())
      log.info("Get Batch status: msgId=" + msgId);

    long start = System.currentTimeMillis();

    SecretGetBatchStatusCmd cmd = new SecretGetBatchStatusCmd(msgId);
    try {
      int status = Services.getInstance().getSecretService().execute(cmd);
      return new SmsXSenderResponse(msgId, 0, status);
    } catch (CommandExecutionException e) {
      log.error(e,e);
      return new SmsXSenderResponse(null, 0, STATUS_SYSTEM_ERROR);
    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  private static SmsXSenderResponse getCalendarMessageStatus(String messageId) {

    int msgId;
    try {
      msgId = Integer.parseInt(messageId.substring(CALENDAR_MSG_ID_PREFIX.length()));
    } catch (Throwable e) {
      return new SmsXSenderResponse(messageId, 0, STATUS_INVALID_MESSAGE_ID);
    }

    final CalendarCheckMessageStatusCmd cmd = new CalendarCheckMessageStatusCmd();
    cmd.setMsgId(msgId);

    try {
      int result = Services.getInstance().getCalendarService().execute(cmd);

      int messageStatus;
      switch (result) {
        case CalendarCheckMessageStatusCmd.MESSAGE_STATUS_NEW:
        case CalendarCheckMessageStatusCmd.MESSAGE_STATUS_PROCESSED:
          messageStatus = STATUS_ACCEPTED;
          break;
        case CalendarCheckMessageStatusCmd.MESSAGE_STATUS_DELIVERED:
          messageStatus = STATUS_DELIVERED;
          break;
        case CalendarCheckMessageStatusCmd.MESSAGE_STATUS_DELIVERY_FAILED:
          messageStatus = STATUS_DELIVERY_ERROR;
          break;
        default:
          messageStatus = STATUS_MESSAGE_NOT_FOUND;
      }

      return new SmsXSenderResponse(messageId, -1, messageStatus);

    } catch (Throwable e) {
      log.error(e,e);
      return new SmsXSenderResponse(messageId, -1, STATUS_SYSTEM_ERROR);
    }
  }

  private static SmsXSenderResponse getSecretMessageStatus(String messageId) {

    int msgId;
    try {
      msgId = Integer.parseInt(messageId.substring(SECRET_MSG_ID_PREFIX.length()));
    } catch (Throwable e) {
      return new SmsXSenderResponse(messageId, 0, STATUS_INVALID_MESSAGE_ID);
    }

    int messageStatus;

    final SecretGetMessageStatusCmd cmd = new SecretGetMessageStatusCmd();
    cmd.setMsgId(msgId);
    try {
      int status = Services.getInstance().getSecretService().execute(cmd);
      switch (status) {
        case SecretGetMessageStatusCmd.MESSAGE_STATUS_NEW:
        case SecretGetMessageStatusCmd.MESSAGE_STATUS_PROCESSED:
          messageStatus = STATUS_ACCEPTED;
          break;
        case SecretGetMessageStatusCmd.MESSAGE_STATUS_DELIVERED:
          messageStatus = STATUS_DELIVERED;
          break;
        case SecretGetMessageStatusCmd.MESSAGE_STATUS_DELIVERY_FAILED:
          messageStatus = STATUS_DELIVERY_ERROR;
          break;
        default:
          messageStatus = STATUS_MESSAGE_NOT_FOUND;
      }
    } catch (CommandExecutionException e) {
      messageStatus = STATUS_SYSTEM_ERROR;
    }

    return new SmsXSenderResponse(messageId, -1, messageStatus);
  }

  private static SmsXSenderResponse getSenderMessageStatus(String messageId) {

    int msgId;
    try {
      msgId = Integer.parseInt(messageId.substring(SENDER_MSG_ID_PREFIX.length()));
    } catch (Throwable e) {
      return new SmsXSenderResponse(messageId, 0, STATUS_INVALID_MESSAGE_ID);
    }

    final SenderGetMessageStatusCmd cmd = new SenderGetMessageStatusCmd();
    cmd.setMsgId(msgId);
    int messageStatus;
    try {
      int status = Services.getInstance().getSenderService().execute(cmd);
      switch (status) {
        case SenderGetMessageStatusCmd.MESSAGE_STATUS_NEW:
        case SenderGetMessageStatusCmd.MESSAGE_STATUS_PROCESSED:
          messageStatus = STATUS_ACCEPTED;
          break;
        case SenderGetMessageStatusCmd.MESSAGE_STATUS_DELIVERED:
          messageStatus = STATUS_DELIVERED;
          break;
        case SenderGetMessageStatusCmd.MESSAGE_STATUS_DELIVERY_FAILED:
          messageStatus = STATUS_DELIVERY_ERROR;
          break;
        default:
          messageStatus = STATUS_MESSAGE_NOT_FOUND;
      }
    } catch (CommandExecutionException e) {
      messageStatus = STATUS_SYSTEM_ERROR;
    }

    return new SmsXSenderResponse(messageId, -1, messageStatus);
  }



  private String getBannerForAbonent(final String abonentAddress) {
    try {
      return advertisingClient.getBanner(advertisingClientName, abonentAddress);
    } catch (AdvertisingClientException e) {
      log.error("Can't get banner for abonent " + abonentAddress, e);
      return null;
    }
  }

  private String getBannerForAbonent(final String abonentAddress, int maxBannerLength) {
    try {
      return advertisingClient.getBanner(advertisingClientName, abonentAddress, maxBannerLength);
    } catch (AdvertisingClientException e) {
      log.error("Can't get banner for abonent " + abonentAddress, e);
      return null;
    }
  }

  private class ConfigChecker extends IterativeWorker {

    private final File configFile;

    ConfigChecker(File configFile, int checkInterval) {
      super(log, checkInterval);

      this.configFile = configFile;
    }

    public void iterativeWork() {
      try {
        final PropertiesConfig cfg = new PropertiesConfig();
        cfg.load(configFile);
        appendAdvertising = cfg.getBool("append.advertising");
        if (log.isInfoEnabled())
          log.info("Append advertising = " + appendAdvertising);

      } catch (ConfigException e) {
        log.error(e,e);
      }
    }

    protected void stopCurrentWork() {
    }
}

}