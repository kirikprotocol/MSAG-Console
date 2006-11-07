package ru.sibinco.otasme.engine;

import java.util.Vector;

/**
 * User: artem
 * Date: Jul 25, 2006
 *
 * Simple blocking queue
 */

final class Commands2Send {
  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(Commands2Send.class);

  private final Vector list;
  private boolean isEmpty = true;
  private final int maxQueueSize;

  public Commands2Send(final int maxQueueSize) {
    this.list = new Vector();
    this.maxQueueSize = maxQueueSize;
  }

  public final synchronized void put(final Command2Send obj) throws QueueOverflowException {
    synchronized(list) {
      if (list.size() > maxQueueSize)
        throw new QueueOverflowException();
      list.add(obj);
      isEmpty = false;
    }
    notifyAll();
  }

  public final synchronized Vector getCommands() {
    if (isEmpty) {
      try {
        wait();
      } catch (InterruptedException e) {
        Log.error(Commands2Send.class.getName() + ": interrupted: " + e);
      }
    }

    synchronized(list) {
      final Vector vect = new Vector(list);
      list.clear();
      isEmpty = true;
      return vect;
    }

  }

  public final synchronized Command2Send getCommand() {
    if (isEmpty) {
      try {
        wait();
      } catch (InterruptedException e) {
        Log.error(Commands2Send.class.getName() + ": interrupted: " + e);
      }
    }

    synchronized(list) {
      final Object obj = list.remove(0);
      isEmpty = list.isEmpty();
      return (Command2Send)obj;
    }

  }

  public final synchronized boolean isEmpty() {
    synchronized(list) {
      return list.isEmpty();
    }
  }

}
