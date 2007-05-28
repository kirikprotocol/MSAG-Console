package ru.sibinco.smsx.services.secret;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.services.ServiceProcessor;
import ru.sibinco.smsx.services.calendar.CalendarService;
import ru.sibinco.smsx.utils.BlockingQueue;

import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

class SecretRequestProcessor extends ServiceProcessor {

  private static final org.apache.log4j.Category log = org.apache.log4j.Category.getInstance(SecretRequestProcessor.class);

  private final BlockingQueue inQueue;

  public SecretRequestProcessor(final BlockingQueue inQueue, final OutgoingQueue outQueue, final Multiplexor multiplexor) {
    super(log, outQueue, multiplexor);
    this.inQueue = inQueue;
  }

  public void iterativeWork() {
    try {
      processMessage((ParsedMessage)inQueue.getObject());
    } catch (Throwable e) {
      log.error("Error", e);
    }
  }

  public void processMessage(final ParsedMessage parsedMessage) {
    final Message message = parsedMessage.getMessage();
    try {
      log.info("=====================================================================================");
      log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

      final SecretRequestParser.ParseResult res = parsedMessage.getParseResult();

      if (message.getDestinationAddress().equals(SecretService.Properties.SERVICE_ADDRESS) && res.getType().equals(SecretRequestParser.ParseResultType.MSG)) {
        log.error("Destination address in secret equals to secret service address: " + SecretService.Properties.SERVICE_ADDRESS + ". Sends notification");
        sendResponse(message, Data.ESME_RX_P_APPN);
        sendMessage(CalendarService.Properties.SERVICE_ADDRESS, message.getSourceAddress(), SecretService.Properties.WRONG_DESTINATION_ADDRESS);
        return;
      }

      if (res.getType().equals(SecretRequestParser.ParseResultType.ON)) {
        log.info("It is request to register abonent");
        enableService(message, res.getMessage());
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.OFF)) {
        log.info("It is request to unregister abonent");
        disableService(message);
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.MSG)) {
        log.info("It is request to send message");
        addMessageToDB(message, res.getMessage());
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.PWD)) {
        log.info("It is request to deliver messages");
        sendMessagesFromDB(message, res.getMessage());
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.CHANGE_PWD)) {
        log.info("It is request to change password");
        changePassword(message, res.getMessage());
      } else {
        log.info("Unknown message format!!! Bug!!!");
      }

    } catch (Exception e) {
      log.error(e);
      sendResponse(message, Data.ESME_RSYSERR);
    }
  }

  private void enableService(final Message incomingMessage, final String password)  {
    final String abonent = incomingMessage.getSourceAddress();
    String responseMessageText;

    try {
      // Check abonent's registration
      if (SecretUser.loadByNumber(abonent) != null) { // Abonent has been registered

        log.info("Abonent " + abonent + " has already registered in DB");
        responseMessageText = SecretService.Properties.ABONENT_ALREADY_REGISTERED;

      } else { // Abonent has not been registered

        new SecretUser(abonent, password).save();
        log.info("Abonent " + abonent + " registered in DB");
        responseMessageText = SecretService.Properties.REGISTER_OK_MESSAGE;
      }

    } catch (Exception e) {
      log.error("Can't register abonent " + abonent, e);
      responseMessageText = SecretService.Properties.REGISTER_ERROR_MESSAGE;
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
        log.info("Abonent " + abonent + " and all his messages removed from DB");
        responseMessage = SecretService.Properties.UNREGISTER_OK_MESSAGE;

      } else { // Abonent has not been registered

        log.info("Abonent " + abonent + " not registered in DB");
        responseMessage = SecretService.Properties.ABONENT_NOT_REGISTERED.replaceAll("\\{abonent}", abonent);
      }

    } catch (SQLException e) {
      log.error("Can't unregister abonent " + abonent, e);
      responseMessage = SecretService.Properties.UNREGISTER_ERROR_MESSAGE;
    }

    sendResponse(incomingMessage, Data.ESME_ROK);
    sendMessage(incomingMessage.getDestinationAddress(), incomingMessage.getSourceAddress(), responseMessage);
  }

  // !!! NOTE: This method differs from other. If error it sends ESME_SYSERR response instead of ESME_ROK.
  private void addMessageToDB(final Message incomingMessage, final String message)  {
    final String fromAbonent = incomingMessage.getSourceAddress();
    final String toAbonent = incomingMessage.getDestinationAddress();

    try {
      // Chech fromAbonent's registration
      if (SecretUser.loadByNumber(fromAbonent) == null) { // fromAbonent has not been registered
        log.error("Abonent " + fromAbonent + " not registered in DB.");
        sendResponse(incomingMessage, Data.ESME_RX_P_APPN);
        return;
      }

      // Check toAbonent's registration
      if (SecretUser.loadByNumber(toAbonent) == null) { // toAbonent has not been registered
        log.error("Abonent " + toAbonent + " not registered in DB");
        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(SecretService.Properties.SERVICE_ADDRESS, fromAbonent, SecretService.Properties.DESTINATION_ABONENT_NOT_REGISTERED.replaceAll("\\{dest_abonent}", toAbonent));
        sendMessage(SecretService.Properties.SERVICE_ADDRESS, toAbonent, SecretService.Properties.ABONENT_INVITATION);
        return;
      }

      // Store message
      final SecretMessage secretMessage = new SecretMessage(toAbonent, message, fromAbonent, incomingMessage.getDestAddrSubunit());
      secretMessage.save();

      // Send notification message to toAbonent
      final String informMessage = prepareInformMessage(toAbonent, fromAbonent);
      sendResponse(incomingMessage, Data.ESME_ROK);
      sendMessage(SecretService.Properties.SERVICE_ADDRESS, toAbonent, informMessage);

    } catch (SQLException e) {
      log.error("Can't add message to DB", e);
      sendResponse(incomingMessage, Data.ESME_RSYSERR);
    }
  }

  private void sendMessagesFromDB(final Message incomingMessage, final String password) {
    final String toAbonent = incomingMessage.getSourceAddress();

    try {
      final SecretUser user = SecretUser.loadByNumber(toAbonent);

      // Check user's registration
      if (user == null) { // user has not been registered
        log.error("Abonent " + toAbonent + " not registered in DB");
        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(SecretService.Properties.SERVICE_ADDRESS, toAbonent, SecretService.Properties.ABONENT_NOT_REGISTERED.replaceAll("\\{abonent}", toAbonent));
        return;
      }

      // Check user's password
      if (!user.confirmPassword(password)) { // Password is wrong
        log.error("Password is wrong");
        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(SecretService.Properties.SERVICE_ADDRESS, toAbonent, SecretService.Properties.WRONG_PASSWORD);
        return;
      }

      // Load and send messages for toAbonent
      final List messages = SecretMessage.loadByUser(toAbonent);
      if (messages.isEmpty()) { // No messages has been found

        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(SecretService.Properties.SERVICE_ADDRESS, toAbonent, SecretService.Properties.NO_MESSAGES);

      } else { // Some messages has been found

        for (Iterator iter = messages.iterator(); iter.hasNext();) {
          final SecretMessage msg = (SecretMessage) iter.next();
          msg.remove();

          final String deliveryReport = prepareDeliveryReport(toAbonent, msg.getSendDate());
          sendResponse(incomingMessage, Data.ESME_ROK);
          sendMessage(msg.getFromNumber(), toAbonent, msg.getMessage(), msg.getDestAddressSubunit());
          sendMessage(SecretService.Properties.SERVICE_ADDRESS, msg.getFromNumber(), deliveryReport);
        }
      }
    } catch (Exception e) {
      log.error("Can't send messages from DB", e);
      sendResponse(incomingMessage, Data.ESME_ROK);
      sendMessage(SecretService.Properties.SERVICE_ADDRESS, toAbonent, SecretService.Properties.SYSTEM_ERROR);
    }
  }

  private void changePassword(final Message incomingMessage, final String passwords) {
    final String abonent = incomingMessage.getSourceAddress();

    try {

      final SecretUser user = SecretUser.loadByNumber(abonent);
      // Check abonent's registration
      if (user == null) { // Abonent has not been registered
        log.error("Abonent " + abonent + " not registered in DB");
        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(SecretService.Properties.SERVICE_ADDRESS, abonent, SecretService.Properties.ABONENT_NOT_REGISTERED.replaceAll("\\{abonent}", abonent));
        return;
      }

      if (passwords.indexOf(" ") > 0) {
        final String oldPassword = passwords.substring(0, passwords.indexOf(" ")).trim();
        final String newPassword = passwords.substring(passwords.indexOf(" ")).trim();
        log.info("old = " + oldPassword + ", new = " + newPassword);

        // Check old password
        if (!user.confirmPassword(oldPassword)) {
          log.error("Wrong password");
          sendResponse(incomingMessage, Data.ESME_ROK);
          sendMessage(SecretService.Properties.SERVICE_ADDRESS, abonent, SecretService.Properties.WRONG_PASSWORD);
          return;
        }

        // Update password
        user.updatePassword(newPassword);
        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(SecretService.Properties.SERVICE_ADDRESS, abonent, SecretService.Properties.PASSWORD_CHANGED);
      }

    } catch (Exception e) {
      log.error("Can't change password", e);
      sendResponse(incomingMessage, Data.ESME_ROK);
      sendMessage(SecretService.Properties.SERVICE_ADDRESS, abonent, SecretService.Properties.SYSTEM_ERROR);
    }
  }

  private String prepareInformMessage(final String toAbonent, final String fromAbonent) throws SQLException {
    return SecretService.Properties.INFORM_MESSAGE.replaceAll("\\{from_abonent}", fromAbonent).replaceAll("\\{messages_count}", String.valueOf(SecretMessage.loadByUser(toAbonent).size()));
  }

  private String prepareDeliveryReport(final String toAbonent, final Timestamp sendDate) {
    return SecretService.Properties.DELIVERY_REPORT.replaceAll("\\{to_abonent}", toAbonent).replaceAll("\\{send_date}", sendDate.toString());
  }
}
