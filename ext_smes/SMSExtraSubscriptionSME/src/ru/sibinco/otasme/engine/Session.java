package ru.sibinco.otasme.engine;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.otasme.Sme;
import ru.sibinco.otasme.network.OutgoingObject;
import ru.sibinco.otasme.utils.Utils;

import java.util.Date;
import java.util.Properties;

/**
 * User: artem
 * Date: 19.10.2006
 */

public final class Session {

  private static final Category log = Category.getInstance(Session.class);

  // Messages, sended to abonent
  private static final String ON_ERROR_TEXT;
  private static final String OFF_ERROR_TEXT;
  private static final String INFO_TEXT;
  private static final String UNEXPECTED_MESSAGE_ERROR_TEXT;
  private static final String TIMEOUT_ERROR_TEXT;
  private static final String SMSEXTRA_NUMBER;
  private static final String OTA_NUMBER;
  private static final int MAX_OTA_MESSAGE_REPEATS;

  static {
    final Properties config = Utils.loadConfig("sme.properties");
    ON_ERROR_TEXT = Utils.loadString(config, "session.on.error.text");
    OFF_ERROR_TEXT = Utils.loadString(config, "session.off.error.text");
    INFO_TEXT = Utils.loadString(config, "session.info.text");
    UNEXPECTED_MESSAGE_ERROR_TEXT = Utils.loadString(config, "session.unexpected.message.error.text");
    TIMEOUT_ERROR_TEXT = Utils.loadString(config, "session.timeout.error.text");
    SMSEXTRA_NUMBER = Utils.loadString(config, "session.smsextra.smsc.number");
    OTA_NUMBER = Utils.loadString(config, "sme.engine.ota.number");
    MAX_OTA_MESSAGE_REPEATS = Utils.loadInt(config, "session.max.ota.message.repeats");
  }

  private final String abonentNumber;
  private final String smeAddress;
  private final String smscenterNumber;

  private Date lastRequestDate = new Date();
  private SessionState currentState = new StartState();

  public Session(String abonentNumber, String smeAddress, String smscenterNumber) throws SessionsRegistry.SessionsRegistryException, SessionsRegistry.SessionsRegistryOverflowException {
    this.abonentNumber = abonentNumber;
    this.smscenterNumber = smscenterNumber;
    this.smeAddress = smeAddress;
    SessionsRegistry.registerSession(this);
  }

  public String getId() {
    return abonentNumber;
  }

  public Date getLastRequestDate() {
    return lastRequestDate;
  }

  public void setLastRequestDate(Date lastRequestDate) {
    this.lastRequestDate = lastRequestDate;
  }

  public void processMessage(Message incomingMessage) throws UnexpectedMessageException {
    currentState = currentState.processMessage(incomingMessage);
    if (currentState == null)
      unregisterMyself();
  }

  public void doBeforeUnregisterByTimeout() {
    sendMessage(TIMEOUT_ERROR_TEXT, smeAddress);
  }

  private void unregisterMyself() {
    try {
      SessionsRegistry.unregisterSession(this);
    } catch (SessionsRegistry.SessionsRegistryException e) {
      logError("Can't unregister session: ", e);
    }
  }

  private void logError(String str, Exception e) {
    log.error("SESSION: ID = " + getId() + ". " + str, e);
  }

  private void logInfo(String str) {
    log.error("SESSION: ID = " + getId() + ". " + str);
  }

  private void sendMessage(String msg, String sourceAddr) {
    final Message message = new Message();
    message.setSourceAddress(sourceAddr);
    message.setDestinationAddress(abonentNumber);
    message.setMessageString(msg);
    Sme.outQueue.addOutgoingObject(new OutgoingObject(message));
  }

  // ---------------------------------------------- States -------------------------------------------------------------


  private interface SessionState {
    public SessionState processMessage(Message incomingMessage) throws UnexpectedMessageException;
  }

  private final class StartState implements SessionState {
    // Regexes for parse abonent request
    private final static String ON = "ON|oN|On|on";
    private final static String OFF = "OFF|OFf|OoF|oFF|Off|oFf|ofF|off";

    public SessionState processMessage(Message incomingMessage) throws UnexpectedMessageException {
      if (incomingMessage.getType() != Message.TYPE_DELIVER)
        throw new UnexpectedMessageException();

      final String req = incomingMessage.getMessageString();
      if (req.trim().matches(ON))
        return processOn();
      else if (req.trim().matches(OFF))
        return processOff();

      processOther(incomingMessage);
      return null;
    }

    private SessionState processOn() {
      logInfo("ON message received.");
      return sendSRCommand(SMSEXTRA_NUMBER, ON_ERROR_TEXT);
    }

    private SessionState processOff() {
      logInfo("OFF message received.");
      return sendSRCommand(smscenterNumber, OFF_ERROR_TEXT);
    }

    private String removePlusFromAbonentNumber(String number) {
      return (number.indexOf("+") >= 0) ? number.substring(1) : number;
    }

    private SessionState sendSRCommand(String wtsServiceName, String otaMessage) {
      final Message otaRequest = new Message();
      otaRequest.setType(Message.TYPE_WTS_REQUEST);
      otaRequest.setWtsOperationCode(Message.WTS_OPERATION_CODE_COMMAND);
      otaRequest.setWTSUserId(removePlusFromAbonentNumber(abonentNumber));
      otaRequest.setWTSServiceName(wtsServiceName);
      otaRequest.setSourceAddress(OTA_NUMBER);
      otaRequest.setDestinationAddress(OTA_NUMBER);
      otaRequest.setWtsRequestReference(abonentNumber);
      Sme.outQueue.addOutgoingObject(new OutgoingObject(otaRequest));
      logInfo("Send SR_COMMAND with service name = " + wtsServiceName);
      return new OTAState(otaMessage.replaceAll("%SMSC_NUMBER%", smscenterNumber), wtsServiceName);
    }

    private void processOther(Message incomingMessage) {
      logInfo("Unknown message received: " + incomingMessage.getMessageString());
      sendMessage(INFO_TEXT, incomingMessage.getDestinationAddress());
    }
  }

  private final class OTAState implements SessionState {
    private final String errorText;
    private final String serviceName;
    private int currentMessageRepeats = 0;

    public OTAState(String errorText, String serviceName) {
      this.errorText = errorText;
      this.serviceName = serviceName;
    }
    public synchronized SessionState processMessage(Message incomingMessage) throws UnexpectedMessageException {
      if (incomingMessage.getType() != Message.TYPE_WTS_REQUEST || incomingMessage.getWtsOperationCode() != Message.WTS_OPERATION_CODE_AACK) {
        if (incomingMessage.getType() == Message.TYPE_DELIVER)
          sendMessage(UNEXPECTED_MESSAGE_ERROR_TEXT, smeAddress);
        throw new UnexpectedMessageException();
      }

      logInfo("OTA message received.");
      if (incomingMessage.getWTSErrorCode() != 0) { // Error occured
        logInfo("OTA Platform return error with code = " + incomingMessage.getWTSErrorCode());

        if (incomingMessage.getWtsErrorStatus() == Message.WTS_ERROR_STATUS_PERM) { // Permanent error occured
          logInfo("It is permanent error. Send SMS");
          sendMessage(errorText, smeAddress);

        } else { // Temporary error occured

          logInfo("It is temporary error. Repeat SR_COMMAND after some time.");
          try {
            currentMessageRepeats ++;
            if (currentMessageRepeats < MAX_OTA_MESSAGE_REPEATS) {
              CommandsRepeater.addCommand(new Command2Send(abonentNumber, serviceName, abonentNumber));
              return this;
            } else {
              sendMessage(errorText, smeAddress);
              return null;
            }
          } catch (QueueOverflowException e) {
            logInfo("Commands Repeater is full. Send Error sms");
            sendMessage(errorText, smeAddress);
            return null;
          }

        }

      } else // No error occured
        logInfo("OTA Platform return success");


      return null;
    }
  }

  // ---------------------------------------------- Exceptions ---------------------------------------------------------


  public final static class UnexpectedMessageException extends Exception {}
}
