package ru.sibinco.smsx.network.smppnetwork;

import com.eyeline.sme.utils.queue.delayed.DelayedQueue;


/**
 * User: artem
 * Date: 20.02.2007
 */

public class SMPPIncomingQueue extends DelayedQueue {

  public void putInObj(final SMPPTransportObject inObj) {
    addObject(inObj);
  }

  public SMPPTransportObject getInObj() {
    return (SMPPTransportObject)removeNextObject();
  }
}