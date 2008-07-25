package ru.sibinco.smsx.engine.service.calendar.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarMessage;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class CalendarCheckMessageStatusCmd extends Command {

  public static final int MESSAGE_STATUS_UNKNOWN = -1;
  public static final int MESSAGE_STATUS_NEW = CalendarMessage.STATUS_NEW;
  public static final int MESSAGE_STATUS_PROCESSED = CalendarMessage.STATUS_PROCESSED;
  public static final int MESSAGE_STATUS_DELIVERED = CalendarMessage.STATUS_DELIVERED;
  public static final int MESSAGE_STATUS_DELIVERY_FAILED = CalendarMessage.STATUS_DELIVERY_FAILED;

  private int msgId;

  public int getMsgId() {
    return msgId;
  }

  public void setMsgId(int msgId) {
    this.msgId = msgId;
  }

  public interface Receiver {
    public int execute(CalendarCheckMessageStatusCmd cmd) throws CommandExecutionException;
  }
}
