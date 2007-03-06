package ru.sibinco.otasme.engine;

import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.otasme.Sme;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.otasme.network.OutgoingObject;
import ru.sibinco.otasme.network.IncomingObject;
import ru.sibinco.otasme.utils.ConnectionPool;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Date;

/**
 * User: artem
 * Date: 19.10.2006
 */

public final class Session {

  private static final Category log = Category.getInstance(Session.class);

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
    sendMessage(SmeProperties.Session.TIMEOUT_ERROR_TEXT, smeAddress);
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

  private void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      Sme.multiplexor.sendResponse(msg);
      log.debug("Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      log.warn("Exception occured sending delivery response.", e);
    }
  }

  // ---------------------------------------------- States -------------------------------------------------------------


  private interface SessionState {
    public SessionState processMessage(Message incomingMessage) throws UnexpectedMessageException;
  }

  private final class StartState implements SessionState {
    // Regexes for parse abonent request
    private final static String ON = "ON|oN|On|on|\\s*";
    private final static String OFF = "OFF|OFf|OoF|oFF|Off|oFf|ofF|off";

    public SessionState processMessage(Message incomingMessage) throws UnexpectedMessageException {
      if (incomingMessage.getType() != Message.TYPE_DELIVER)
        throw new UnexpectedMessageException();

      final String req = incomingMessage.getMessageString();
      if (req.trim().matches(ON))
        return processOn(incomingMessage);
      else if (req.trim().matches(OFF))
        return processOff(incomingMessage);

      processOther(incomingMessage);
      return null;
    }

    private SessionState processOn(Message incomingMessage) {
      logInfo("ON message received.");
      try {
        if (checkAbonentSubscription()) { // Abonent already registered
          sendResponse(incomingMessage, Data.ESME_ROK);
          sendMessage(SmeProperties.Session.ABONENT_ALREADY_REGISTERED, smeAddress);
          return null;
        }
        sendResponse(incomingMessage, Data.ESME_ROK);
        return sendSRCommand(SmeProperties.Session.SMSEXTRA_NUMBER, SmeProperties.Session.ON_ERROR_TEXT, true);

      } catch (SQLException e) {
        log.error(e);
        sendResponse(incomingMessage, Data.ESME_RSYSERR);
        sendMessage(SmeProperties.Session.INTERNAL_ERROR, smeAddress);
        return null;
      }
    }

    private SessionState processOff(Message incomingMessage) {
      logInfo("OFF message received.");

      try {
        if (!checkAbonentSubscription()) { // Abonent not registered
          sendResponse(incomingMessage, Data.ESME_ROK);
          sendMessage(SmeProperties.Session.ABONENT_NOT_REGISTERED, smeAddress);
          return null;
        }
        sendResponse(incomingMessage, Data.ESME_ROK);
        return sendSRCommand(smscenterNumber, SmeProperties.Session.OFF_ERROR_TEXT, false);

      } catch (SQLException e) {
        log.error(e);
        sendResponse(incomingMessage, Data.ESME_RSYSERR);
        sendMessage(SmeProperties.Session.INTERNAL_ERROR, smeAddress);
        return null;
      }
    }

    private String removePlusFromAbonentNumber(String number) {
      return (number.indexOf("+") >= 0) ? number.substring(1) : number;
    }

    private SessionState sendSRCommand(String wtsServiceName, String otaMessage, boolean enableService) {
      final Message otaRequest = new Message();
      otaRequest.setType(Message.TYPE_WTS_REQUEST);
      otaRequest.setWtsOperationCode(Message.WTS_OPERATION_CODE_COMMAND);
      otaRequest.setWTSUserId(removePlusFromAbonentNumber(abonentNumber));
      otaRequest.setWTSServiceName(wtsServiceName);
      otaRequest.setSourceAddress(SmeProperties.Session.OTA_NUMBER);
      otaRequest.setDestinationAddress(SmeProperties.Session.OTA_NUMBER);
      otaRequest.setWtsRequestReference(abonentNumber);
      Sme.outQueue.addOutgoingObject(new OutgoingObject(otaRequest));
      logInfo("Send SR_COMMAND with service name = " + wtsServiceName);
//      final Message message = new Message();
//      message.setType(Message.TYPE_WTS_REQUEST);
//      message.setSourceAddress(abonentNumber);
//      message.setWtsRequestReference(abonentNumber);
//      message.setWtsOperationCode(Message.WTS_OPERATION_CODE_AACK);
//      message.setWTSErrorCode(0);
//      Sme.inQueue.addIncomingObject(new IncomingObject(message));
      return new OTAState(otaMessage.replaceAll("%SMSC_NUMBER%", smscenterNumber), wtsServiceName, enableService);
    }

    private void processOther(Message incomingMessage) {
      logInfo("Unknown message received: " + incomingMessage.getMessageString());
      sendResponse(incomingMessage, Data.ESME_ROK);
      sendMessage(SmeProperties.Session.INFO_TEXT, incomingMessage.getDestinationAddress());
    }

    private boolean checkAbonentSubscription() throws SQLException {
      Connection conn = null;
      PreparedStatement ps = null;
      ResultSet rs = null;

      try {
        conn = ConnectionPool.getConnection();
        ps = conn.prepareStatement(SmeProperties.Session.FIND_ABONENT_SQL);
        ps.setString(1, abonentNumber);
        rs = ps.executeQuery();

        return rs.next();

      } finally {
        try {
          if (rs != null)
            rs.close();
          if (ps != null)
            ps.close();
          if (conn != null)
            conn.close();
        } catch (SQLException e) {
          log.error(e);
        }
      }
    }
  }

  private final class OTAState implements SessionState {
    private final String errorText;
    private final String serviceName;
    private int currentMessageRepeats = 0;
    private final boolean enableService;

    public OTAState(String errorText, String serviceName, boolean enableService) {
      this.errorText = errorText;
      this.serviceName = serviceName;
      this.enableService = enableService;
    }

    public synchronized SessionState processMessage(Message incomingMessage) throws UnexpectedMessageException {
      if (incomingMessage.getType() != Message.TYPE_WTS_REQUEST || incomingMessage.getWtsOperationCode() != Message.WTS_OPERATION_CODE_AACK) {
        if (incomingMessage.getType() == Message.TYPE_DELIVER)
          sendMessage(SmeProperties.Session.UNEXPECTED_MESSAGE_ERROR_TEXT, smeAddress);
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
            if (currentMessageRepeats < SmeProperties.Session.MAX_OTA_MESSAGE_REPEATS) {
              CommandsRepeater.addCommand(new Command2Send(abonentNumber, serviceName, abonentNumber));
              return this;
            } else
              sendMessage(errorText, smeAddress);

          } catch (QueueOverflowException e) {
            logInfo("Commands Repeater is full. Send Error sms");
            sendMessage(errorText, smeAddress);
          }
        }

      } else  // No error occured
        logInfo("OTA Platform return success");

      processUser(abonentNumber, enableService);
      return null;
    }

    private void processUser(String abonentNumber, boolean enableService) {
      Connection conn = null;
      PreparedStatement ps = null;
      ResultSet rs = null;

      try {
        ps = conn.prepareStatement(SmeProperties.Session.FIND_ABONENT_SQL);
        ps.setString(1, abonentNumber);
        rs = ps.executeQuery();

        if (rs.next()) { // User already in DB
          if (!enableService) {
            ps = conn.prepareStatement(SmeProperties.Session.DELETE_ABONENT_SQL);
            ps.setString(1, abonentNumber);
            ps.executeUpdate();
          }
        } else if (enableService) {
          ps = conn.prepareStatement(SmeProperties.Session.ADD_ABONENT_SQL);
          ps.setString(1, abonentNumber);
          ps.executeUpdate();
        }

      } catch (SQLException e) {
        log.error(e);
      } finally {
        try {
          if (rs != null)
            rs.close();
          if (ps != null)
            ps.close();
          if (conn != null)
            conn.close();
        } catch (SQLException e) {
          log.error(e);
        }
      }
    }
  }



  // ---------------------------------------------- Exceptions ---------------------------------------------------------


  public final static class UnexpectedMessageException extends Exception {}
}
