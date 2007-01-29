package ru.sibinco.smsx.services;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.network.OutgoingObject;
import ru.sibinco.smsx.network.OutgoingQueue;

/**
 * User: artem
 * Date: 24.01.2007
 */

public abstract class InternalService {

  private final Category log;
  protected OutgoingQueue outQueue;
  protected Multiplexor multiplexor;
  protected String serviceAddress;

  protected InternalService(Category log) {
    this.log = log;
  }

  protected void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      multiplexor.sendResponse(msg);
      log.debug("Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      log.warn("Exception occured sending delivery response.", e);
    }
  }

  protected void sendMessage(final String fromAbonent, final String toAbonent, final String message) {
    log.info("Send message from abonent: " + fromAbonent + "; to abonent: " + toAbonent + "; message: " + message);
    final Message msg = new Message();
    msg.setSourceAddress(fromAbonent);
    msg.setDestinationAddress(toAbonent);
    msg.setMessageString(message);
    outQueue.addOutgoingObject(new OutgoingObject(msg));
  }

  /**
   * Process incoming message
   * @param message
   * @return true, if message has correct format, false if message has wrong format
   */
  public abstract boolean processMessage(Message message) throws ProcessException;

  /**
   * Initiate service (properties, threads, ...)
   * @param info
   * @throws InitializationException
   */
  public void init(InitializationInfo info) throws InitializationException {
    if (info.getOutQueue() == null)
      throw new InitializationException("Out queue not specified");
    if (info.getMultiplexor() == null)
      throw new InitializationException("Multiplexor not specified");

    outQueue = info.getOutQueue();
    multiplexor = info.getMultiplexor();
    serviceAddress = info.getServiceAddress();
  }

  public abstract String getName();
}
