package ru.sibinco.smsx.network.smppnetwork;

import com.eyeline.sme.utils.queue.delayed.DelayedQueue;
import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 22.02.2007
 */

public class SMPPOutgoingQueue extends DelayedQueue {

  public synchronized void addOutgoingObject(final SMPPTransportObject outObj) {
    addObject(outObj);
  }

  public synchronized SMPPTransportObject removeNextOutgoingObject() {
    return (SMPPTransportObject)removeNextObject();
  }

  synchronized void restoreOutgoingObject(final int connectionId, final int sequenceId) {
    final SMPPTransportObject outObj = removeOutgoingObject(connectionId, sequenceId);
    if (outObj != null) {
      outObj.setNextTime(-1);
      addOutgoingObject(outObj);
    }
  }

  synchronized SMPPTransportObject removeOutgoingObject(final int connectionId, final int sequenceId) {
    return (SMPPTransportObject)removeFirstEquals(new Comparable() {
      public int compareTo(Object o) {
        Message message = ((SMPPTransportObject)o).getOutgoingMessage();
        return (message != null && message.getConnectionId() == connectionId && message.getSequenceNumber() == sequenceId) ? 0 : 1;
      }
    });

  }

  synchronized void removeOutgoingObject(final SMPPTransportObject object) {
    if (object == null)
      return;

    remove(object);
  }
}
