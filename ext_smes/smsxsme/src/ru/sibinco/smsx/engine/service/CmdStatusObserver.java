package ru.sibinco.smsx.engine.service;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class CmdStatusObserver implements CommandObserver {

  private final int[] statuses;
  private boolean shouldWait = true;

  public CmdStatusObserver(int[] statuses) {
    this.statuses = statuses;
  }

  public synchronized void update(Command command) {
    if (statuses == null) {
      shouldWait = false;
      notifyAll();
      return;
    }
    
    for (int i=0; i<statuses.length; i++)
      if (command.getStatus() == statuses[i]) {
        shouldWait = false;
        notifyAll();
      }
  }

  public synchronized void waitStatus() {
    try {
      if (shouldWait)
        wait();
    } catch (InterruptedException e) {
    }
  }
}
