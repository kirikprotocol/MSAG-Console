package ru.sibinco.calendarsme.engine.secret;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.calendarsme.SmeProperties;
import ru.sibinco.calendarsme.network.OutgoingObject;
import ru.sibinco.calendarsme.network.OutgoingQueue;

import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class SecretRequestProcessor {

  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(SecretRequestProcessor.class);

  private final String smeAddress;

  private final OutgoingQueue outQueue;
  private final Multiplexor multiplexor;

  public SecretRequestProcessor(final OutgoingQueue outQueue, final Multiplexor multiplexor) {
    this.outQueue = outQueue;
    this.multiplexor = multiplexor;
    smeAddress = SmeProperties.General.SECRET_ENGINE_SME_ADDRESS;
  }

  public boolean processMessage(final Message message) {
    if (message == null)
      return false;
    try {
      final SecretRequestParser.ParseResult res = SecretRequestParser.parseRequest(message.getMessageString());

      Log.info("=====================================================================================");
      Log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

      if (res.getType().equals(SecretRequestParser.ParseResultType.ON) && message.getDestinationAddress().equals(smeAddress)) {
        Log.info("It is request to register abonent");
        enableService(message, res.getMessage());
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.OFF) && message.getDestinationAddress().equals(smeAddress)) {
        Log.info("It is request to unregister abonent");
        disableService(message);
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.MSG) && !message.getDestinationAddress().equals(smeAddress)) {
        Log.info("It is request to send message");
        addMessageToDB(message, res.getMessage());
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.PWD) && message.getDestinationAddress().equals(smeAddress)) {
        Log.info("It is request to deliver messages");
        sendMessagesFromDB(message, res.getMessage());
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.CHANGE_PWD) && message.getDestinationAddress().equals(smeAddress)) {
        Log.info("It is request to change password");
        changePassword(message, res.getMessage());
      } else {
        Log.error("Request not recognized.");
        return false;
      }

    } catch (SecretRequestParser.WrongMessageFormatException e) {
      return false;
    } catch (Exception e) {
      Log.error("", e);
      return false;
    }

    return true;
  }

  private void enableService(final Message incomingMessage, final String password)  {
    final String abonent = incomingMessage.getSourceAddress();
    String responseMessageText;

    try {
      // Check abonent's registration
      if (SecretUser.loadByNumber(abonent) != null) { // Abonent has been registered

        Log.info("Abonent " + abonent + " has already registered in DB");
        responseMessageText = SmeProperties.General.SECRET_ENGINE_ABONENT_ALREADY_REGISTERED;

      } else { // Abonent has not been registered

        new SecretUser(abonent, password).save();
        Log.info("Abonent " + abonent + " registered in DB");
        responseMessageText = SmeProperties.General.SECRET_ENGINE_REGISTER_OK_MESSAGE;
      }

    } catch (Exception e) {
      Log.error("Can't register abonent " + abonent, e);
      responseMessageText = SmeProperties.General.SECRET_ENGINE_REGISTER_ERROR_MESSAGE;
    }

    sendResponse(incomingMessage, Data.ESME_ROK);
    sendMessage(incomingMessage.getDestinationAddress(), abonent, responseMessageText);
  }

  private void disableService(final Message incomingMessage) {
    final String abonent = incomingMessage.getSourceAddress();
    String responseMessage;

    try {
      // Check abonent's registration
      if (SecretUser.loadByNumber(abonent) != null) { // Abonent has been registered in DB

        // Remove messages
        final List messages = SecretMessage.loadByUser(abonent);
        for (Iterator iter = messages.iterator(); iter.hasNext();)
          ((SecretMessage) iter.next()).remove();
        // Remove user
        new SecretUser(abonent, null).remove();
        Log.info("Abonent " + abonent + " and all his messages removed from DB");
        responseMessage = SmeProperties.General.SECRET_ENGINE_UNREGISTER_OK_MESSAGE;

      } else { // Abonent has not been registered

        Log.info("Abonent " + abonent + " not registered in DB");
        responseMessage = SmeProperties.General.SECRET_ENGINE_ABONENT_NOT_REGISTERED.replaceAll("\\{abonent}", abonent);
      }

    } catch (SQLException e) {
      Log.error("Can't unregister abonent " + abonent, e);
      responseMessage = SmeProperties.General.SECRET_ENGINE_UNREGISTER_ERROR_MESSAGE;
    }

    sendResponse(incomingMessage, Data.ESME_ROK);
    sendMessage(incomingMessage.getDestinationAddress(), incomingMessage.getSourceAddress(), responseMessage);
  }

  private void addMessageToDB(final Message incomingMessage, final String message)  {
    final String fromAbonent = incomingMessage.getSourceAddress();
    final String toAbonent = incomingMessage.getDestinationAddress();

    try {
      // Chech fromAbonent's registration
      if (SecretUser.loadByNumber(fromAbonent) == null) { // fromAbonent has not been registered
        Log.error("Abonent " + fromAbonent + " not registered in DB");
        sendResponse(incomingMessage, Data.ESME_RSYSERR);
        // We don't need to send message to fromAbonent here!!! Because SMSC impl.
        return;
      }

      // Check toAbonent's registration
      if (SecretUser.loadByNumber(toAbonent) == null) { // toAbonent has not been registered
        Log.error("Abonent " + toAbonent + " not registered in DB");
        sendResponse(incomingMessage, Data.ESME_RX_P_APPN);
        sendMessage(smeAddress, fromAbonent, SmeProperties.General.SECRET_ENGINE_DESTINATION_ABONENT_NOT_REGISTERED.replaceAll("\\{dest_abonent}", toAbonent));
        return;
      }

      // Store message
      final SecretMessage secretMessage = new SecretMessage(toAbonent, message, fromAbonent);
      secretMessage.save();

      // Send notification message to toAbonent
      final String informMessage = prepareInformMessage(toAbonent, fromAbonent);
      sendResponse(incomingMessage, Data.ESME_ROK);
      sendMessage(smeAddress, toAbonent, informMessage);

    } catch (SQLException e) {
      Log.error("Can't add message to DB", e);
      sendResponse(incomingMessage, Data.ESME_RSYSERR);
      // We don't need to send message to fromAbonent here!!! Because SMSC impl.
    }
  }

  private void sendMessagesFromDB(final Message incomingMessage, final String password) {
    final String toAbonent = incomingMessage.getSourceAddress();

    try {
      final SecretUser user = SecretUser.loadByNumber(toAbonent);

      // Check user's registration
      if (user == null) { // user has not been registered
        Log.error("Abonent " + toAbonent + " not registered in DB");
        sendResponse(incomingMessage, Data.ESME_RX_P_APPN);
        sendMessage(smeAddress, toAbonent, SmeProperties.General.SECRET_ENGINE_DESTINATION_ABONENT_NOT_REGISTERED.replaceAll("\\{abonent}", toAbonent));
        return;
      }

      // Check user's password
      if (!user.confirmPassword(password)) { // Password is wrond
        Log.error("Password is wrong");
        sendResponse(incomingMessage, Data.ESME_RX_P_APPN);
        sendMessage(smeAddress, toAbonent, SmeProperties.General.SECRET_ENGINE_WRONG_PASSWORD);
        return;
      }

      // Load and send messages for toAbonent
      final List messages = SecretMessage.loadByUser(toAbonent);
      if (messages.isEmpty()) { // No messages has been found

        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(smeAddress, toAbonent, SmeProperties.General.SECRET_ENGINE_NO_MESSAGES);

      } else { // Some messages has been found

        for (Iterator iter = messages.iterator(); iter.hasNext();) {
          final SecretMessage msg = (SecretMessage) iter.next();
          msg.remove();

          final String deliveryReport = prepareDeliveryReport(toAbonent, msg.getSendDate());
          sendResponse(incomingMessage, Data.ESME_ROK);
          sendMessage(msg.getFromNumber(), toAbonent, msg.getMessage());
          sendMessage(smeAddress, msg.getFromNumber(), deliveryReport);
        }
      }
    } catch (Exception e) {
      Log.error("Can't send messages from DB", e);
      sendResponse(incomingMessage, Data.ESME_RSYSERR);
      sendMessage(smeAddress, toAbonent, SmeProperties.General.SECRET_ENGINE_SYSTEM_ERROR);
    }
  }

  private void changePassword(final Message incomingMessage, final String passwords) {
    final String abonent = incomingMessage.getSourceAddress();

    try {

      final SecretUser user = SecretUser.loadByNumber(abonent);
      // Check abonent's registration
      if (user == null) { // Abonent has not been registered
        Log.error("Abonent " + abonent + " not registered in DB");
        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(smeAddress, abonent, SmeProperties.General.SECRET_ENGINE_ABONENT_NOT_REGISTERED.replaceAll("\\{abonent}", abonent));
        return;
      }

      if (passwords.indexOf(" ") > 0) {
        final String oldPassword = passwords.substring(0, passwords.indexOf(" ")).trim();
        final String newPassword = passwords.substring(passwords.indexOf(" ")).trim();
        Log.info("old = " + oldPassword + ", new = " + newPassword);

        // Check old password
        if (!user.confirmPassword(oldPassword)) {
          Log.error("Wrong password");
          sendResponse(incomingMessage, Data.ESME_ROK);
          sendMessage(smeAddress, abonent, SmeProperties.General.SECRET_ENGINE_WRONG_PASSWORD);
          return;
        }

        // Update password
        user.updatePassword(newPassword);
        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(smeAddress, abonent, SmeProperties.General.SECRET_ENGINE_PASSWORD_CHANGED);
      }

    } catch (Exception e) {
      Log.error("Can't change password", e);
      sendResponse(incomingMessage, Data.ESME_ROK);
      sendMessage(smeAddress, abonent, SmeProperties.General.SECRET_ENGINE_SYSTEM_ERROR);
    }
  }

  private void sendMessage(final String fromAbonent, final String toAbonent, final String message) {
    Log.info("Send message from abonent: " + fromAbonent + "; to abonent: " + toAbonent + "; message: " + message);
    final Message msg = new Message();
    msg.setSourceAddress(fromAbonent);
    msg.setDestinationAddress(toAbonent);
    msg.setMessageString(message);
    outQueue.addOutgoingObject(new OutgoingObject(msg));
  }

  private String prepareInformMessage(final String toAbonent, final String fromAbonent) throws SQLException {
    return SmeProperties.General.SECRET_ENGINE_INFORM_MESSAGE.replaceAll("\\{from_abonent}", fromAbonent).replaceAll("\\{messages_count}", String.valueOf(SecretMessage.loadByUser(toAbonent).size()));
  }

  private String prepareDeliveryReport(final String toAbonent, final Timestamp sendDate) {
    return SmeProperties.General.SECRET_ENGINE_DELIVERY_REPORT.replaceAll("\\{to_abonent}", toAbonent).replaceAll("\\{send_date}", sendDate.toString());
  }

  private void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      multiplexor.sendResponse(msg);
      Log.info("Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      Log.warn("Exception occured sending delivery response.", e);
    }
  }

}
