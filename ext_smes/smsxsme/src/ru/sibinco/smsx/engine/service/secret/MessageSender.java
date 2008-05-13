package ru.sibinco.smsx.engine.service.secret;

import com.eyeline.sme.utils.ds.DataSourceException;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SubmitResponse;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretDataSource;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretMessage;
import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;

import java.sql.Timestamp;

/**
 * User: artem
 * Date: 04.07.2007
 */

class MessageSender {

  private static final Category log = Category.getInstance("SECRET");

  private final SecretDataSource ds;
  private final SMPPOutgoingQueue outQueue;

  private String serviceAddress;
  private String msgDestinationAbonentInform;
  private String msgDestinationAbonentInvitation;
  private String msgDeliveryReport;

  MessageSender(SecretDataSource ds, SMPPOutgoingQueue outQueue) {
    this.ds = ds;
    this.outQueue = outQueue;
  }

  public void setMsgDestinationAbonentInform(String msgDestinationAbonentInform) {
    this.msgDestinationAbonentInform = msgDestinationAbonentInform;
  }

  public void setMsgDestinationAbonentInvitation(String msgDestinationAbonentInvitation) {
    this.msgDestinationAbonentInvitation = msgDestinationAbonentInvitation;
  }

  public void setMsgDeliveryReport(String msgDeliveryReport) {
    this.msgDeliveryReport = msgDeliveryReport;
  }

  public void setServiceAddress(String serviceAddress) {
    this.serviceAddress = serviceAddress;
  }

  public void sendInformMessage(SecretMessage secretMessage) {
    try {
      sendMessage(serviceAddress, secretMessage.getDestinationAddress(), prepareInformMessage(secretMessage.getDestinationAddress(), secretMessage.getSourceAddress()), "smsx");
    } catch (DataSourceException e) {
      log.error("Can't send inform message", e);
    }
  }

  public void sendInvitationMessage(String destinationAddress) {
    sendMessage(serviceAddress, destinationAddress, msgDestinationAbonentInvitation, "smsx");
  }

  public void sendSecretMessage(SecretMessage message) {

    final Message outMsg = new Message();
    outMsg.setSourceAddress(message.getSourceAddress());
    outMsg.setDestinationAddress(message.getDestinationAddress());
    outMsg.setDestAddrSubunit(message.getDestAddressSubunit());
    outMsg.setMessageString(message.getMessage());
    outMsg.setConnectionName(message.getConnectionName());
    if (message.isSaveDeliveryStatus())
      outMsg.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);

    final SecretTransportObject outObj = new SecretTransportObject(message);
    outObj.setOutgoingMessage(outMsg);
    outQueue.addOutgoingObject(outObj);

    message.setStatus(SecretMessage.STATUS_PROCESSED);
    try {
      ds.saveSecretMessage(message);
    } catch (DataSourceException e) {
      log.error("Can't save secret message", e);
    }

    // Send notification to originator
    if (message.isNotifyOriginator())
      sendMessage(serviceAddress, message.getSourceAddress(), prepareDeliveryReport(message.getDestinationAddress(), message.getSendDate()), message.getConnectionName());
  }

  private void sendMessage(String sourceAddress, String destinationAddress, String msg, String connectionName) {
    final Message notificationMessage = new Message();
    notificationMessage.setSourceAddress(sourceAddress);
    notificationMessage.setDestinationAddress(destinationAddress);
    notificationMessage.setMessageString(msg);
    notificationMessage.setConnectionName(connectionName);
    final SMPPTransportObject outObj1 = new SMPPTransportObject();
    outObj1.setOutgoingMessage(notificationMessage);
    outQueue.addOutgoingObject(outObj1);
  }

  private String prepareDeliveryReport(final String toAbonent, final Timestamp sendDate) {
    return msgDeliveryReport.replaceAll("\\{to_abonent}", toAbonent).replaceAll("\\{send_date}", sendDate.toString());
  }

  private String prepareInformMessage(final String toAbonent, final String fromAbonent) throws DataSourceException {
//    return msgDestinationAbonentInform.replaceAll("\\{from_abonent}", fromAbonent).replaceAll("\\{messages_count}", String.valueOf(ds.loadMessagesCountByAddress(toAbonent)));
    return msgDestinationAbonentInform;
  }


  private class SecretTransportObject extends SMPPTransportObject {
    private final SecretMessage msg;

    SecretTransportObject(SecretMessage msg) {
      this.msg = msg;
    }

    public void handleResponse(PDU pdu) {
      try {
        if (msg.isSaveDeliveryStatus()) {
          if (pdu.getStatusClass() != PDU.STATUS_CLASS_NO_ERROR) {
            msg.setStatus(SecretMessage.STATUS_DELIVERY_FAILED);
            msg.setSmppStatus(pdu.getStatus());
            ds.saveSecretMessage(msg);
          } else {
            msg.setSmppId(Long.parseLong(((SubmitResponse)pdu).getMessageId()));
            ds.updateMessageSmppId(msg);
          }
        }
      } catch (DataSourceException e) {
        log.error("Can't save delivery status!");
      }
    }

    public void handleSendError() {
      try {
        if (msg.isSaveDeliveryStatus()) {
          msg.setStatus(SecretMessage.STATUS_DELIVERY_FAILED);
          ds.saveSecretMessage(msg);
        }
      } catch (DataSourceException e) {
        log.error("Can't save secret message", e);
      }
    }
  }
}
