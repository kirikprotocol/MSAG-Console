package ru.sibinco.smsx.engine.service;

import java.util.Collection;
import java.util.LinkedList;
import java.util.Iterator;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class Command {

  // Statuses
  public static final int STATUS_SUCCESS = 0;
  public static final int STATUS_SYSTEM_ERROR = 1;
  public static final int STATUS_INT = 2;

  private int status = STATUS_SUCCESS;

  private final Collection observers;


  protected Command() {
    observers = new LinkedList();
  }

  public int getStatus() {
    return status;
  }

  public void addExecutionObserver(CommandObserver observer) {
    observers.add(observer);
  }

  protected void notifyObservers() {
    for (Iterator iter = observers.iterator(); iter.hasNext();)
      ((CommandObserver)iter.next()).update(this);
  }

  public void update(int status) {
    this.status = status;
    notifyObservers();
  }

}
