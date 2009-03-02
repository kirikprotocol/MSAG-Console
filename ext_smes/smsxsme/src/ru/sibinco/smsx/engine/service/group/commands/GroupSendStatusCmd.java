package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.group.DeliveryStatus;

/**
 * User: artem
 * Date: 15.07.2008
 */

public class GroupSendStatusCmd extends Command {

  private int msgId;

  public int getMsgId() {
    return msgId;
  }

  public GroupSendStatusCmd setMsgId(int msgId) {
    this.msgId = msgId;
    return this;
  }  

  public interface Receiver {
    public DeliveryStatus[] execute(GroupSendStatusCmd cmd) throws CommandExecutionException;
  }
}
