package ru.sibinco.calendarsme.engine.secret;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.calendarsme.network.OutgoingObject;
import ru.sibinco.calendarsme.network.OutgoingQueue;
import ru.sibinco.calendarsme.utils.MessageEncoder;
import ru.sibinco.calendarsme.utils.Utils;

import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Iterator;
import java.util.List;
import java.util.Properties;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class SecretRequestProcessor {

  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(SecretRequestProcessor.class);

  private final String informMessage;
  private final String noMessagesMessage;
  private final String deliveryReport;
  private final String smeAddress;

  private final OutgoingQueue outQueue;
  private final Multiplexor multiplexor;

  public SecretRequestProcessor(final Properties config, final OutgoingQueue outQueue, final Multiplexor multiplexor) {
    this.informMessage = Utils.loadString(config, "secret.engine.inform.message");
    this.noMessagesMessage = Utils.loadString(config, "secret.engine.no.messages");
    this.deliveryReport = Utils.loadString(config, "secret.engine.delivery.report");
    this.smeAddress = Utils.loadString(config, "secret.engine.sme.address");

    this.outQueue = outQueue;
    this.multiplexor = multiplexor;
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
        enableService(message.getSourceAddress(), res.getMessage());
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.OFF) && message.getDestinationAddress().equals(smeAddress)) {
        Log.info("It is request to unregister abonent");
        disableService(message.getSourceAddress());
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.MSG) && !message.getDestinationAddress().equals(smeAddress)) {
        Log.info("It is request to send message");
        addMessageToDB(message.getSourceAddress(), message.getDestinationAddress(), res.getMessage());
      } else if (res.getType().equals(SecretRequestParser.ParseResultType.PWD) && message.getDestinationAddress().equals(smeAddress)) {
        Log.info("It is request to deliver messages");
        sendMessagesFromDB(message.getSourceAddress(), res.getMessage());
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

  private void enableService(final String abonent, final String password) throws SQLException, MessageEncoder.EncodeException {
    new SecretUser(abonent, password).save();
    Log.info("Abonent " + abonent + " registered in DB");
  }

  private void disableService(final String abonent) throws SQLException {
    // Remove messages
    final List messages = SecretMessage.loadByUser(abonent);
    for (Iterator iter = messages.iterator(); iter.hasNext();)
      ((SecretMessage)iter.next()).remove();
    // Remove user
    new SecretUser(abonent, null).remove();
    Log.info("Abonent " + abonent + " and all his messages removed from DB");
  }

  private void addMessageToDB(final String fromAbonent, final String toAbonent, final String message) throws SQLException, ReceiverNotRegisteredException, SenderNotRegisteredException {
    if (SecretUser.loadByNumber(toAbonent) == null)
      throw new ReceiverNotRegisteredException();
    if (SecretUser.loadByNumber(fromAbonent) == null)
      throw new SenderNotRegisteredException();

    final SecretMessage secretMessage = new SecretMessage(toAbonent, message, fromAbonent);
    secretMessage.save();

    sendMessage(smeAddress, toAbonent, prepareInformMessage(toAbonent, fromAbonent));
  }

  private void sendMessagesFromDB(final String toAbonent, final String password) throws SQLException, MessageEncoder.EncodeException, IncorrectPasswordException, ReceiverNotRegisteredException {
    final SecretUser user = SecretUser.loadByNumber(toAbonent);
    if (user == null)
      throw new ReceiverNotRegisteredException();
    if (!user.confirmPassword(password))
      throw new IncorrectPasswordException();

    final List messages = SecretMessage.loadByUser(toAbonent);
    if (messages.isEmpty())
      sendMessage(smeAddress, toAbonent, noMessagesMessage);
    else {
      for (Iterator iter = messages.iterator(); iter.hasNext();) {
        final SecretMessage msg = (SecretMessage)iter.next();
        sendMessage(msg.getFromNumber(), toAbonent, msg.getMessage());
        sendMessage(smeAddress, msg.getFromNumber(), prepareDeliveryReport(toAbonent, msg.getSendDate()));
        msg.remove();
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
    return informMessage.replaceAll("\\{from_abonent}", fromAbonent).replaceAll("\\{messages_count}", String.valueOf(SecretMessage.loadByUser(toAbonent).size()));
  }

  private String prepareDeliveryReport(final String toAbonent, final Timestamp sendDate) {
    return deliveryReport.replaceAll("\\{to_abonent}", toAbonent).replaceAll("\\{send_date}", sendDate.toString());
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

  public class ReceiverNotRegisteredException extends Exception {
  }

  public class SenderNotRegisteredException extends Exception {
  }

  public class IncorrectPasswordException extends Exception {
  }

}
