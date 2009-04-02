package ru.sibinco.smsx.engine.service.sender.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderMessage;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SenderGetMessageStatusCmd extends Command {

  public static final int MESSAGE_STATUS_UNKNOWN = -1;
  public static final int MESSAGE_STATUS_NEW = SenderMessage.STATUS_NEW;
  public static final int MESSAGE_STATUS_PROCESSED = SenderMessage.STATUS_PROCESSED;
  public static final int MESSAGE_STATUS_DELIVERED = SenderMessage.STATUS_DELIVERED;
  public static final int MESSAGE_STATUS_DELIVERY_FAILED = SenderMessage.STATUS_DELIVERY_FAILED;

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
