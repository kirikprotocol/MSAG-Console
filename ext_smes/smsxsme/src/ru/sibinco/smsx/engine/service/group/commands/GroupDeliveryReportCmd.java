package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: 16.07.2008
 */

public class GroupDeliveryReportCmd extends Command {

  private boolean delivered;
  private int umr;
  private long msgId;

  public boolean isDelivered() {
    return delivered;
  }

  public void setDelivered(boolean delivered) {
    this.delivered = delivered;
  }

  public int getUmr() {
    return umr;
  }

  public void setUmr(int umr) {
    this.umr = umr;
  }

  public long getMsgId() {
    return msgId;
  }

  public void setMsgId(long msgId) {
    this.msgId = msgId;
  }

  public interface Receiver {
    public boolean execute(GroupDeliveryReportCmd cmd) throws CommandExecutionException;
  }
}
