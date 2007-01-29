package ru.sibinco.smsx.utils;

import java.util.Vector;

/**
 * User: artem
 * Date: Jul 25, 2006
 *
 * Simple blocking queue
 */

public final class BlockingQueue {
  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(BlockingQueue.class);

  private final Vector list;
  private boolean isEmpty = true;
  private final int maxQueueSize;

  public BlockingQueue(final int maxQueueSize) {
    this.list = new Vector();
    this.maxQueueSize = maxQueueSize;
  }

  public final synchronized void put(final Object obj) throws QueueOverflowException {
    synchronized(list) {
      if (list.size() > maxQueueSize)
        throw new QueueOverflowException();
      list.add(obj);
      isEmpty = false;
    }
    notifyAll();
  }

  public final synchronized Vector getObjects() {
    if (isEmpty) {
      try {
        wait();
      } catch (InterruptedException e) {
        Log.error(BlockingQueue.class.getName() + ": interrupted: " + e);
      }
    }

    synchronized(list) {
      final Vector vect = new Vector(list);
      list.clear();
      isEmpty = true;
      return vect;
    }

  }

  public final synchronized Object getObject() {
    if (isEmpty) {
      try {
        wait();
      } catch (InterruptedException e) {
        Log.error(BlockingQueue.class.getName() + ": interrupted: " + e);
      }
    }

    synchronized(list) {
      final Object obj = list.remove(0);
      isEmpty = list.isEmpty();
      return obj;
    }

  }

  public final synchronized boolean isEmpty() {
    synchronized(list) {
      return list.isEmpty();
    }
  }

  public class QueueOverflowException extends Exception {
  }
}
