package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import com.eyeline.sme.utils.config.ConfigException;
import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.CmdStatusObserver;
import ru.sibinco.smsx.engine.service.ServiceManager;
import ru.sibinco.smsx.engine.service.sender.commands.SenderSendMessageCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.secret.commands.SecretSendMessageCmd;
import ru.sibinco.smsx.engine.service.secret.commands.SecretGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.blacklist.commands.BlackListCheckMsisdnCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarCheckMessageStatusCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.advertising.AdvertisingClientException;
import ru.sibinco.smsx.utils.operators.Operator;
import ru.sibinco.smsx.utils.operators.OperatorsList;

import java.io.File;
import java.rmi.RemoteException;
import java.util.Date;

class SmsXSenderHandler implements SmsXSender {

  private static final Category log = Category.getInstance("SMSXSENDER SOAP");

  private static final String CALENDAR_MSG_ID_PREFIX = "0";
  private static final String SECRET_MSG_ID_PREFIX = "1";
  private static final String SENDER_MSG_ID_PREFIX = "2";

  // Secret service errors
  private static final int STATUS_SECRET_DESTINATION_ABONENT_NOT_REGISTERED = -200;
  private static final int STATUS_SECRET_WRONG_DESTINATION_ADDRESS = -201;

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

  private final OperatorsList operatorsList;
  private final AdvertisingClient advertisingClient;

  SmsXSenderHandler(String configDir, OperatorsList operatorsList, AdvertisingClient advertisingClient) {

    this.operatorsList = operatorsList;
    this.advertisingClient = advertisingClient;

    try {
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "soaphandlers/smsxsendhandler.properties"));
      serviceAddress = config.getString("service.address");
      advertisingClientName = config.getString("advertising.service.name");
      advertisingRestriction = config.getInt("advertising.restriction");
      advertisingDelimiter = config.getString("advertising.delimiter");
    } catch (ConfigException e) {
      throw new SOAPHandlerInitializationException(e);
    }
  }

  public SmsXSenderResponse sendSms(String msisdn, String message, boolean express, boolean secret, boolean calendar, long calendarTimeUTC, boolean advertising) throws RemoteException {
    final long start = System.currentTimeMillis();
    try {
      log.info("Send SMS: dstaddr=" + msisdn + "; msg=" + message + "; express=" + express + "; secret=" + secret + "; calendar=" + calendar + "; time=" + new Date(calendarTimeUTC) + "; advertising=" + advertising);

      int status = 0;
      String id_message = null;

      // Check operator
      final Operator operator = operatorsList.getOperatorByAddress(msisdn);
      if (operator == null || !operator.getName().equals("MTS")) {
        log.error("Unknown or incorrect operator for dstaddr=" + msisdn);
        return new SmsXSenderResponse(null, -1, STATUS_DESTINATION_ABONENT_UNKNOWN);
      }

      // Check black list
      final BlackListCheckMsisdnCmd cmd = new BlackListCheckMsisdnCmd();
      cmd.setMsisdn(msisdn);
      final CmdStatusObserver statusObserver = new CmdStatusObserver(new int[] {BlackListCheckMsisdnCmd.STATUS_SUCCESS, BlackListCheckMsisdnCmd.STATUS_SYSTEM_ERROR});
      cmd.addExecutionObserver(statusObserver);
      ServiceManager.getInstance().getBlackListService().execute(cmd);
      statusObserver.waitStatus();

      if (cmd.getStatus() == BlackListCheckMsisdnCmd.STATUS_SYSTEM_ERROR) {
        log.error("Can't check dstaddr in blacklist. dstaddr=" + msisdn);
        return new SmsXSenderResponse(null, -1, STATUS_SYSTEM_ERROR);
      } else if (cmd.isInBlackList()) {
        log.error("Dstaddr=" + msisdn + " is in black list");
        return new SmsXSenderResponse(null, -1, STATUS_DESTINATION_ABONENT_IN_BLACK_LIST);
      }

      // Prepare message
      if (advertising) {
        final String banner = (advertisingRestriction > 0) ? getBannerForAbonent(msisdn, advertisingRestriction - message.length()) : getBannerForAbonent(msisdn);
        log.info("Append banner: " + banner);
        if (banner != null)
          message += advertisingDelimiter + banner;
      }

      if (calendar) {
        final CalendarSendMessageCmd c = new CalendarSendMessageCmd();
        c.setSourceAddress(serviceAddress);
        c.setDestinationAddress(msisdn);
        c.setSendDate(new Date(calendarTimeUTC));
        c.setMessage(message);
        c.setDestAddressSubunit(express ? 1 : -1);
        c.setStoreDeliveryStatus(true);
        final CmdStatusObserver observer = new CmdStatusObserver(null);
        c.addExecutionObserver(observer);

        ServiceManager.getInstance().getCalendarService().execute(c);
        observer.waitStatus();

        switch(c.getStatus()) {
          case CalendarSendMessageCmd.STATUS_SUCCESS:
            status = STATUS_ACCEPTED;
            id_message = CALENDAR_MSG_ID_PREFIX + c.getMsgId();
            break;
          case CalendarSendMessageCmd.STATUS_WRONG_SEND_DATE:
            status = STATUS_CALENDAR_WRONG_SEND_DATE;
            break;
          case CalendarSendMessageCmd.STATUS_WRONG_DESTINATION_ADDRESS:
            status = STATUS_CALENDAR_WRONG_DESTINATION_ADDRESS;
            break;
          default:
            status = STATUS_SYSTEM_ERROR;

        }

      } else if (secret) {
        final SecretSendMessageCmd c = new SecretSendMessageCmd();
        c.setSourceAddress(serviceAddress);
        c.setDestinationAddress(msisdn);
        c.setMessage(message);
        c.setDestAddressSubunit(express ? 1 : -1);
        c.setSaveDeliveryStatus(true);
        c.setNotifyOriginator(false);
        final CmdStatusObserver observer = new CmdStatusObserver(null);
        c.addExecutionObserver(observer);
        ServiceManager.getInstance().getSecretService().execute(c);
        observer.waitStatus();
        switch(c.getStatus()) {
          case SecretSendMessageCmd.STATUS_SUCCESS:
            status = STATUS_ACCEPTED;
            id_message = SECRET_MSG_ID_PREFIX + c.getMsgId();
            break;
          case SecretSendMessageCmd.STATUS_DESTINATION_ABONENT_NOT_REGISTERED:
            status = STATUS_SECRET_DESTINATION_ABONENT_NOT_REGISTERED;
            break;
          case SecretSendMessageCmd.STATUS_DESTINATION_ADDRESS_IS_NOT_ALLOWED:
            status = STATUS_SECRET_WRONG_DESTINATION_ADDRESS;
            break;
          default:
            status = STATUS_SYSTEM_ERROR;
        }

      } else {
        SenderSendMessageCmd c = new SenderSendMessageCmd();
        c.setSourceAddress(serviceAddress);
        c.setDestinationAddress(msisdn);
        c.setMessage(message);
        c.setDestAddressSubunit(express ? 1 : -1);
        c.setStorable(true);
        final CmdStatusObserver observer = new CmdStatusObserver(null);
        c.addExecutionObserver(observer);
        ServiceManager.getInstance().getSenderService().execute(c);
        observer.waitStatus();
        switch (c.getStatus()) {
          case SenderSendMessageCmd.STATUS_SUCCESS:
            status = STATUS_ACCEPTED;
            id_message = SENDER_MSG_ID_PREFIX + c.getMsgId();
            break;
          default:
            status = STATUS_SYSTEM_ERROR;
        }
      }

      return new SmsXSenderResponse(id_message, 0, status);
    } catch (Throwable e) {
      return new SmsXSenderResponse(null, -1, STATUS_SYSTEM_ERROR);

    } finally {
      log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }

  public SmsXSenderResponse checkStatus(String messageId) throws RemoteException {
    final long start = System.currentTimeMillis();
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
      log.info("Time=" + (System.currentTimeMillis() - start));
    }

    return new SmsXSenderResponse(messageId, smppStatus, messageStatus);
  }


  private static SmsXSenderResponse getCalendarMessageStatus(String messageId) {

    int msgId;
    try {
      msgId = Integer.parseInt(messageId.substring(CALENDAR_MSG_ID_PREFIX.length()));
    } catch (Throwable e) {
      return new SmsXSenderResponse(messageId, 0, STATUS_INVALID_MESSAGE_ID);
    }

    int messageStatus = STATUS_SYSTEM_ERROR;
    int smppStatus = -1;

    final CalendarCheckMessageStatusCmd cmd = new CalendarCheckMessageStatusCmd();
    cmd.setMsgId(msgId);
    final CmdStatusObserver o = new CmdStatusObserver(null);
    cmd.addExecutionObserver(o);
    ServiceManager.getInstance().getCalendarService().execute(cmd);
    o.waitStatus();

    if (cmd.getStatus() == CalendarCheckMessageStatusCmd.STATUS_SUCCESS) {
      switch (cmd.getMessageStatus()) {
        case CalendarCheckMessageStatusCmd.MESSAGE_STATUS_NEW:
        case CalendarCheckMessageStatusCmd.MESSAGE_STATUS_PROCESSED:
          messageStatus = STATUS_ACCEPTED;
          break;
        case CalendarCheckMessageStatusCmd.MESSAGE_STATUS_DELIVERED:
          messageStatus = STATUS_DELIVERED;
          smppStatus = cmd.getSmppStatus();
          break;
        case CalendarCheckMessageStatusCmd.MESSAGE_STATUS_DELIVERY_FAILED:
          messageStatus = STATUS_DELIVERY_ERROR;
          smppStatus = cmd.getSmppStatus();
          break;
        default:
          messageStatus = STATUS_MESSAGE_NOT_FOUND;
      }

    }

    return new SmsXSenderResponse(messageId, smppStatus, messageStatus);
  }

  private static SmsXSenderResponse getSecretMessageStatus(String messageId) {

    int msgId;
    try {
      msgId = Integer.parseInt(messageId.substring(SECRET_MSG_ID_PREFIX.length()));
    } catch (Throwable e) {
      return new SmsXSenderResponse(messageId, 0, STATUS_INVALID_MESSAGE_ID);
    }

    int messageStatus = STATUS_SYSTEM_ERROR;
    int smppStatus = -1;

    final SecretGetMessageStatusCmd cmd = new SecretGetMessageStatusCmd();
    cmd.setMsgId(msgId);
    final CmdStatusObserver o = new CmdStatusObserver(null);
    cmd.addExecutionObserver(o);
    ServiceManager.getInstance().getSecretService().execute(cmd);
    o.waitStatus();
    if (cmd.getStatus() == SecretGetMessageStatusCmd.STATUS_SUCCESS) {

      switch (cmd.getMessageStatus()) {
        case SecretGetMessageStatusCmd.MESSAGE_STATUS_NEW:
        case SecretGetMessageStatusCmd.MESSAGE_STATUS_PROCESSED:
          messageStatus = STATUS_ACCEPTED;
          break;
        case SecretGetMessageStatusCmd.MESSAGE_STATUS_DELIVERED:
          messageStatus = STATUS_DELIVERED;
          smppStatus = cmd.getSmppStatus();
          break;
        case SecretGetMessageStatusCmd.MESSAGE_STATUS_DELIVERY_FAILED:
          messageStatus = STATUS_DELIVERY_ERROR;
          smppStatus = cmd.getSmppStatus();
          break;
        default:
          messageStatus = STATUS_MESSAGE_NOT_FOUND;
      }

    }

    return new SmsXSenderResponse(messageId, smppStatus, messageStatus);
  }

  private static SmsXSenderResponse getSenderMessageStatus(String messageId) {

    int msgId;
    try {
      msgId = Integer.parseInt(messageId.substring(SENDER_MSG_ID_PREFIX.length()));
    } catch (Throwable e) {
      return new SmsXSenderResponse(messageId, 0, STATUS_INVALID_MESSAGE_ID);
    }

    int messageStatus = STATUS_SYSTEM_ERROR;
    int smppStatus = -1;

    final SenderGetMessageStatusCmd cmd = new SenderGetMessageStatusCmd();
    cmd.setMsgId(msgId);
    final CmdStatusObserver o = new CmdStatusObserver(null);
    cmd.addExecutionObserver(o);
    ServiceManager.getInstance().getSenderService().execute(cmd);
    o.waitStatus();

    if (cmd.getStatus() == SenderGetMessageStatusCmd.STATUS_SUCCESS) {

      switch (cmd.getMessageStatus()) {
        case SenderGetMessageStatusCmd.MESSAGE_STATUS_NEW:
        case SenderGetMessageStatusCmd.MESSAGE_STATUS_PROCESSED:
          messageStatus = STATUS_ACCEPTED;
          break;
        case SenderGetMessageStatusCmd.MESSAGE_STATUS_DELIVERED:
          messageStatus = STATUS_DELIVERED;
          smppStatus = cmd.getSmppStatus();
          break;
        case SenderGetMessageStatusCmd.MESSAGE_STATUS_DELIVERY_FAILED:
          messageStatus = STATUS_DELIVERY_ERROR;
          smppStatus = cmd.getSmppStatus();
          break;
        default:
          messageStatus = STATUS_MESSAGE_NOT_FOUND;
      }

    }

    return new SmsXSenderResponse(messageId, smppStatus, messageStatus);
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

}
