package ru.sibinco.smsx.services;

import ru.aurorisoft.smpp.Multiplexor;
import ru.sibinco.smsx.network.OutgoingQueue;

/**
 * User: artem
 * Date: 25.01.2007
 */
public class InitializationInfo {
  private final Multiplexor multiplexor;
  private final OutgoingQueue outQueue;
  private final String serviceAddress;

  public InitializationInfo(Multiplexor multiplexor, OutgoingQueue outQueue, String serviceNumber) {
    this.multiplexor = multiplexor;
    this.outQueue = outQueue;
    this.serviceAddress = serviceNumber;
  }

  public Multiplexor getMultiplexor() {
    return multiplexor;
  }

  public OutgoingQueue getOutQueue() {
    return outQueue;
  }

  public String getServiceAddress() {
    return serviceAddress;
  }
}
