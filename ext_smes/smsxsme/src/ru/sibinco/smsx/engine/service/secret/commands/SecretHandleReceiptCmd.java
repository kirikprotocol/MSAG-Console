package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: 13.05.2008
 */

public class SecretHandleReceiptCmd extends Command {
  private boolean delivered;
  private long smppMessageId;

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

  public interface Receiver {
    public boolean execute(SecretHandleReceiptCmd cmd);
  }
}
