package ru.sibinco.smsx.engine.service;

import java.util.Collection;
import java.util.ArrayList;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class AsyncCommand extends Command {

  // Statuses
  public static final int STATUS_SUCCESS = 0;
  public static final int STATUS_SYSTEM_ERROR = 1;
  public static final int STATUS_INT = 2;

  private int status = STATUS_SUCCESS;

  private final Collection<CommandObserver> observers;

  protected AsyncCommand() {
    observers = new ArrayList<CommandObserver>(10);
  }

  public int getStatus() {
    return status;
  }

  public void addExecutionObserver(CommandObserver observer) {
    observers.add(observer);
  }

  protected void notifyObservers() {
    for (CommandObserver observer : observers) observer.update(this);
  }

  public void update(int status) {
    this.status = status;
    notifyObservers();
  }

  public interface AsyncCommandReceiver<CommandClass extends AsyncCommand> {
    public void execute(CommandClass cmd) throws CommandExecutionException;
  }
}
