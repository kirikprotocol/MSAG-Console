package ru.sibinco.smsx.engine.service.sender.commands;

import ru.sibinco.smsx.engine.service.Command;
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
  private int smppStatus = -1;
  private int messageStatus = -1;

  public int getMsgId() {
    return msgId;
  }

  public void setMsgId(int msgId) {
    this.msgId = msgId;
  }

  public int getSmppStatus() {
    return smppStatus;
  }

  public void setSmppStatus(int smppStatus) {
    this.smppStatus = smppStatus;
  }

  public int getMessageStatus() {
    return messageStatus;
  }

  public void setMessageStatus(int messageStatus) {
    this.messageStatus = messageStatus;
  }

  public interface Receiver {
    public void execute(SenderGetMessageStatusCmd cmd);
  }
}
