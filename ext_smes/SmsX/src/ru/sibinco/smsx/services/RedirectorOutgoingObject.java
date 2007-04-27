package ru.sibinco.smsx.services;

import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.network.OutgoingObject;

/**
 * User: artem
 * Date: 26.01.2007
 */

public class RedirectorOutgoingObject extends OutgoingObject{

  private static final Category log = Category.getInstance(RedirectorOutgoingObject.class);

  private final Multiplexor multiplexor;
  private final int sequenceNumber;

  public RedirectorOutgoingObject(Message outgoingMessage, Multiplexor multiplexor) {
    super(outgoingMessage);
    this.multiplexor = multiplexor;
    this.sequenceNumber = outgoingMessage.getSequenceNumber();
  }

  public void changeStatus(int newStatus) {
    if (newStatus == DELIVER_OK)
      sendResponse(getOutgoingMessage(), Data.ESME_ROK);
    else if (newStatus == DELIVER_PERM_ERROR)
      sendResponse(getOutgoingMessage(), PDU.STATUS_CLASS_PERM_ERROR);
  }

  private void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      log.error("SEQUENCE NUMBER: " + msg.getSequenceNumber() + " " + sequenceNumber);
      log.error("MESSAGE: " + msg.getMessageString());
      msg.setSequenceNumber(sequenceNumber);
      multiplexor.sendResponse(msg);
      log.debug("Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      log.error(e);
    }
  }

}
