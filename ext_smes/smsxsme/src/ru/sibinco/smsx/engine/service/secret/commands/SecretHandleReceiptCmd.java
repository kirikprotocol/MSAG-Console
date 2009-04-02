package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: 13.05.2008
 */

public class SecretHandleReceiptCmd extends Command {
  
  private boolean delivered;
  private long smppMessageId;
  private int umr;

  public long getSmppMessageId() {
    return smppMessageId;
  }

  public void setSmppMessageId(long smppMessageId) {
    this.smppMessageId = smppMessageId;
  }

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

  public interface Receiver {
    public boolean execute(SecretHandleReceiptCmd cmd) throws CommandExecutionException;
  }
}
