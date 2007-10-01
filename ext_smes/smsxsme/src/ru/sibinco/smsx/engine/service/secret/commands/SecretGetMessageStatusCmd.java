package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;
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
  private int messageStatus;
  private int smppStatus;

  public int getMsgId() {
    return msgId;
  }

  public void setMsgId(int msgId) {
    this.msgId = msgId;
  }

  public int getMessageStatus() {
    return messageStatus;
  }

  public void setMessageStatus(int messageStatus) {
    this.messageStatus = messageStatus;
  }

  public int getSmppStatus() {
    return smppStatus;
  }

  public void setSmppStatus(int smppStatus) {
    this.smppStatus = smppStatus;
  }

  public interface Receiver {
    public void execute(SecretGetMessageStatusCmd cmd);
  }
}
