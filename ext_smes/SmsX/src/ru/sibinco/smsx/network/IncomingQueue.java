package ru.sibinco.smsx.network;


import ru.sibinco.smsx.utils.Utils;

import java.util.LinkedList;
import java.util.List;
import java.util.Properties;

public class IncomingQueue {

  protected final static org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(IncomingQueue.class);

  private final List incomingQueue = new LinkedList();
  private boolean isQueueEmpty = true;
  private int maxQueueSize = 100;
  private boolean lockByThrottledError = false;
  private boolean lockByOutgoingQueueSize = false;

  public IncomingQueue(Properties config) {
    maxQueueSize = Utils.loadInt(config, "incoming.queue.max.size");

    if (maxQueueSize < 100)
      Log.warn("\"incoming.queue.max.size\" property value is less then minimum, use 100 by default.");
  }

  public synchronized void addIncomingObject(IncomingObject obj) {
    if (obj == null)
      return;

    synchronized(incomingQueue) {
      incomingQueue.add(obj);

      isQueueEmpty = false;
      notify();
    }
  }

  public synchronized void check(String abonentAddress)
    throws QueueOverflowException, UserQueueOverflowException, ThrottlingException {
    if (lockByThrottledError)
      throw new ThrottlingException("Throttling error (ESME has exceeded allowed message limits).");
    if (lockByOutgoingQueueSize)
      throw new QueueOverflowException("Outgoing queue full, incoming queue locked.");

    synchronized(incomingQueue) {
      if (incomingQueue.size() >= maxQueueSize)
        throw new QueueOverflowException("IncomingQueue full.");
    }

  }

  public synchronized IncomingObject getIncomingObject() {
    if (isQueueEmpty) {
      try {
        wait();
      } catch (InterruptedException e) {
        Log.error("Incoming queue: interrupted: " + e);
      }
    }

    synchronized(incomingQueue) {
      final IncomingObject inObj =  (IncomingObject) incomingQueue.remove(0);
      isQueueEmpty = incomingQueue.isEmpty();
      return inObj;
    }
  }

  public boolean isEmpty() {
    synchronized(incomingQueue) {
      return incomingQueue.size() == 0;
    }
  }

  protected int getMaxQueueSize() {
    return maxQueueSize;
  }

  protected synchronized void lockByThrottledError(boolean flag) {
    lockByThrottledError = flag;
  }

  protected synchronized void setLockByOutgoingQueueSize(boolean flag) {
    lockByOutgoingQueueSize = flag;
  }

  public synchronized void checkQueueLength(IncomingObject obj)
    throws QueueOverflowException, UserQueueOverflowException {
    if (obj == null)
      return;

    synchronized(incomingQueue) {
      if (incomingQueue.size() >= maxQueueSize)
        throw new QueueOverflowException("IncomingQueue full.");
    }

  }

}
