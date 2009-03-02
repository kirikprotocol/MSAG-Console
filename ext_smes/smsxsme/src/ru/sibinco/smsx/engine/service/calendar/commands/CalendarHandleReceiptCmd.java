package ru.sibinco.smsx.engine.service.calendar.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 13.05.2008
 */

public class CalendarHandleReceiptCmd extends Command {

  private boolean delivered;
  private int umr;

  public int getUmr() {
    return umr;
  }

  public void setUmr(int umr) {
    this.umr = umr;
  }

  public boolean isDelivered() {
    return delivered;
  }

  public void setDelivered(boolean delivered) {
    this.delivered = delivered;
  }

  public interface Receiver {
    public boolean execute(CalendarHandleReceiptCmd cmd) throws CommandExecutionException;
  }
}
