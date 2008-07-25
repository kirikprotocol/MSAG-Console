package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.secret.datasource.SecretMessage;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SecretGetMessageStatusCmd extends Command {

  public static final int MESSAGE_STATUS_UNKNOWN = -1;
  public static final int MESSAGE_STATUS_NEW = SecretMessage.STATUS_NEW;
  public static final int MESSAGE_STATUS_PROCESSED = SecretMessage.STATUS_PROCESSED;
  public static final int MESSAGE_STATUS_DELIVERED = SecretMessage.STATUS_DELIVERED;
  public static final int MESSAGE_STATUS_DELIVERY_FAILED = SecretMessage.STATUS_DELIVERY_FAILED;

  private int msgId;

  public int getMsgId() {
    return msgId;
  }

  public void setMsgId(int msgId) {
    this.msgId = msgId;
  }

  public interface Receiver {
    public int execute(SecretGetMessageStatusCmd cmd) throws CommandExecutionException;
  }
}
