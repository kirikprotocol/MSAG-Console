package ru.sibinco.smsx.engine.service.sender;

import com.eyeline.sme.utils.ds.DataSourceException;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderDataSource;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderMessage;
import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;

/**
 * User: artem
 * Date: 06.07.2007
 */

class MessageSender {

  private static final Category log = Category.getInstance("SENDER");

  private final SMPPOutgoingQueue outQueue;
  private final SenderDataSource ds;

  MessageSender(SMPPOutgoingQueue outQueue, SenderDataSource ds) {
    this.outQueue = outQueue;
    this.ds = ds;
  }

  public void sendMessage(SenderMessage message) {
    final Message msg = new Message();
    msg.setSourceAddress(message.getSourceAddress());
    msg.setDestinationAddress(message.getDestinationAddress());
    msg.setMessageString(message.getMessage());
    msg.setDestAddrSubunit(message.getDestAddrSubunit());
    msg.setConnectionName(message.getConnectionName());

    final SenderSMPPTransportObject outObj = new SenderSMPPTransportObject(message);
    outObj.setOutgoingMessage(msg);
    outQueue.addOutgoingObject(outObj);

    message.setStatus(SenderMessage.STATUS_PROCESSED);
  }


  private class SenderSMPPTransportObject extends SMPPTransportObject {
    private final SenderMessage msg;

    SenderSMPPTransportObject(SenderMessage senderMessage) {
      this.msg = senderMessage;
    }

    public void handleResponse(PDU pdu) {
      if (msg.isStorable()) {
        try {
          msg.setStatus(pdu.getStatusClass() == PDU.STATUS_CLASS_NO_ERROR ? SenderMessage.STATUS_DELIVERED : SenderMessage.STATUS_DELIVERY_FAILED);
          msg.setSmppStatus(pdu.getStatus());
          ds.saveSenderMessage(msg);
        } catch (DataSourceException e) {
          log.error("Can't save Sender Message", e);
        }
      }
    }

    public void handleSendError() {
      if (msg.isStorable()) {
        try {
          msg.setStatus(SenderMessage.STATUS_DELIVERY_FAILED);
          ds.saveSenderMessage(msg);
        } catch (DataSourceException e) {
          log.error("Can't save Sender Message", e);
        }
      }
    }
  }

}
