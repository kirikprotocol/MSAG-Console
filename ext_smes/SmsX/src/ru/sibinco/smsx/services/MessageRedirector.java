package ru.sibinco.smsx.services;

import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.sibinco.smsx.network.OutgoingQueue;

/**
 * User: artem
 * Date: 26.01.2007
 */
public final class MessageRedirector {
  public static void redirectMessage(Message message, String toAddress, OutgoingQueue outQueue, Multiplexor multiplexor) {
    message.setDestinationAddress(toAddress);
    outQueue.addOutgoingObject(new RedirectorOutgoingObject(message, multiplexor));
  }
}
