package ru.sibinco.calendarsme.engine.secret;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.calendarsme.SmeProperties;
import ru.sibinco.calendarsme.network.OutgoingObject;
import ru.sibinco.calendarsme.network.OutgoingQueue;
import ru.sibinco.calendarsme.utils.MessageEncoder;

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
      } else {
        Log.error("Request not recognized.");
        return false;
      }

    } catch (SecretRequestParser.ParseException e) {
      Log.error("", e);
      sendResponse(message, Data.ESME_RSYSERR);
    } catch (SQLException e) {
      Log.error("", e);
      sendResponse(message, Data.ESME_RSYSERR);
    } catch (MessageEncoder.EncodeException e) {
      Log.error("", e);
      sendResponse(message, Data.ESME_RSYSERR);
    } catch (IncorrectPasswordException e) {
      Log.error("Wrong password!", e);
      sendResponse(message, Data.ESME_RX_P_APPN);
    } catch (ReceiverNotRegisteredException e) {
      Log.error("Abonent " + message.getDestinationAddress() + " not registered in DB!", e);
      sendResponse(message, Data.ESME_RX_P_APPN);
    } catch (SenderNotRegisteredException e) {
      Log.error("Abonent " + message.getSourceAddress() + " not registered in DB!", e);
      sendResponse(message, Data.ESME_RX_P_APPN);
    } catch (SecretRequestParser.WrongMessageFormatException e) {
      return false;
    }
    return true;
  }

  private void enableService(final Message incomingMessage, final String password) throws MessageEncoder.EncodeException {
    final String abonent = incomingMessage.getSourceAddress();
    String responseMessageText;

    try {
      new SecretUser(abonent, password).save();
      Log.info("Abonent " + abonent + " registered in DB");
      responseMessageText = SmeProperties.General.SECRET_ENGINE_REGISTER_OK_MESSAGE;
    } catch (SQLException e) {
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
      // Remove messages
      final List messages = SecretMessage.loadByUser(abonent);
      for (Iterator iter = messages.iterator(); iter.hasNext();)
        ((SecretMessage) iter.next()).remove();
      // Remove user
      new SecretUser(abonent, null).remove();

      Log.info("Abonent " + abonent + " and all his messages removed from DB");
      responseMessage = SmeProperties.General.SECRET_ENGINE_UNREGISTER_OK_MESSAGE;
    } catch (SQLException e) {
      Log.error("Can't unregister abonent " + abonent, e);
      responseMessage = SmeProperties.General.SECRET_ENGINE_UNREGISTER_ERROR_MESSAGE;
    }

    sendResponse(incomingMessage, Data.ESME_ROK);
    sendMessage(incomingMessage.getDestinationAddress(), incomingMessage.getSourceAddress(), responseMessage);
  }

  private void addMessageToDB(final Message incomingMessage, final String message) throws SQLException, ReceiverNotRegisteredException, SenderNotRegisteredException {
    final String fromAbonent = incomingMessage.getSourceAddress();
    final String toAbonent = incomingMessage.getDestinationAddress();
    if (SecretUser.loadByNumber(toAbonent) == null)
      throw new ReceiverNotRegisteredException();
    if (SecretUser.loadByNumber(fromAbonent) == null)
      throw new SenderNotRegisteredException();

    final SecretMessage secretMessage = new SecretMessage(toAbonent, message, fromAbonent);
    secretMessage.save();

    sendResponse(incomingMessage, Data.ESME_ROK);
    sendMessage(smeAddress, toAbonent, prepareInformMessage(toAbonent, fromAbonent));
  }

  private void sendMessagesFromDB(final Message incomingMessage, final String password) throws SQLException, MessageEncoder.EncodeException, IncorrectPasswordException, ReceiverNotRegisteredException {
    final String toAbonent = incomingMessage.getSourceAddress();
    final SecretUser user = SecretUser.loadByNumber(toAbonent);
    if (user == null)
      throw new ReceiverNotRegisteredException();
    if (!user.confirmPassword(password))
      throw new IncorrectPasswordException();

    final List messages = SecretMessage.loadByUser(toAbonent);
    if (messages.isEmpty()) {
      sendResponse(incomingMessage, Data.ESME_ROK);
      sendMessage(smeAddress, toAbonent, SmeProperties.General.SECRET_ENGINE_NO_MESSAGES);
    } else {
      for (Iterator iter = messages.iterator(); iter.hasNext();) {
        final SecretMessage msg = (SecretMessage)iter.next();
        msg.remove();

        sendResponse(incomingMessage, Data.ESME_ROK);
        sendMessage(msg.getFromNumber(), toAbonent, msg.getMessage());
        sendMessage(smeAddress, msg.getFromNumber(), prepareDeliveryReport(toAbonent, msg.getSendDate()));
      }
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

  public class ReceiverNotRegisteredException extends Exception {}

  public class SenderNotRegisteredException extends Exception {}

  public class IncorrectPasswordException extends Exception {}

}
