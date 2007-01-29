package ru.sibinco.smsx.services;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.network.OutgoingObject;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.utils.Service;

/**
 * User: artem
 * Date: 25.01.2007
 */
public abstract class ServiceProcessor extends Service{

  protected final Category Log;
  protected final OutgoingQueue outQueue;
  protected final Multiplexor multiplexor;

  protected ServiceProcessor(Category log, OutgoingQueue outQueue, Multiplexor multiplexor) {
    super(log);
    Log = log;
    this.outQueue = outQueue;
    this.multiplexor = multiplexor;
  }

  protected void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      multiplexor.sendResponse(msg);
      Log.debug("Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      Log.warn("Exception occured sending delivery response.", e);
    }
  }

  protected void sendMessage(final String fromAbonent, final String toAbonent, final String message) {
    Log.info("Send message from abonent: " + fromAbonent + "; to abonent: " + toAbonent + "; message: " + message);
    final Message msg = new Message();
    msg.setSourceAddress(fromAbonent);
    msg.setDestinationAddress(toAbonent);
    msg.setMessageString(message);
    outQueue.addOutgoingObject(new OutgoingObject(msg));
  }
}
