package ru.sibinco.smsx.engine.service.sender.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.sender.SenderService;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SenderGetMessageStatusCmd extends Command {

  public static final int MESSAGE_STATUS_UNKNOWN = -1;
  public static final int MESSAGE_STATUS_NEW = 0;
  public static final int MESSAGE_STATUS_PROCESSED = 1;
  public static final int MESSAGE_STATUS_DELIVERED = 2;
  public static final int MESSAGE_STATUS_DELIVERY_FAILED = 3;

  private int msgId = -1;

  public int getMsgId() {
    return msgId;
  }

  public void setMsgId(int msgId) {
    this.msgId = msgId;
  }

  public interface Receiver {
    public int execute(SenderGetMessageStatusCmd cmd) throws CommandExecutionException;
  }
}
