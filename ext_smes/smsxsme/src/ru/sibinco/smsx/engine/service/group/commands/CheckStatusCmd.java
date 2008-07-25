package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: 15.07.2008
 */

public class CheckStatusCmd extends Command {

  private long msgId;

  public long getMsgId() {
    return msgId;
  }

  public CheckStatusCmd setMsgId(long msgId) {
    this.msgId = msgId;
    return this;
  }  

  public interface Receiver {
    public MessageStatus execute(CheckStatusCmd cmd) throws CommandExecutionException;
  }

  public enum MessageStatus {
    ACCEPTED, DELIVERED, LIST_NOT_FOUND, OWNER_NOT_FOUND, ACCESS_DENIED, SYS_ERR
  }
}
