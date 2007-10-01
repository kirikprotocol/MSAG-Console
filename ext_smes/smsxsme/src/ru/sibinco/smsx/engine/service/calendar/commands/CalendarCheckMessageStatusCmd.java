package ru.sibinco.smsx.engine.service.calendar.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.calendar.datasource.CalendarMessage;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class CalendarCheckMessageStatusCmd extends Command {

  public static final int MESSAGE_STATUS_UNKNOWN = -1;
  public static final int MESSAGE_STATUS_NEW = CalendarMessage.STATUS_NEW;
  public static final int MESSAGE_STATUS_PROCESSED = CalendarMessage.STATUS_PROCESSED;
  public static final int MESSAGE_STATUS_DELIVERED = CalendarMessage.STATUS_DELIVERED;
  public static final int MESSAGE_STATUS_DELIVERY_FAILED = CalendarMessage.STATUS_DELIVERY_FAILED;

  private int msgId;
  private int messageStatus;
  private int smppStatus;
  private boolean saveDeliveryStatus;

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

  public boolean isSaveDeliveryStatus() {
    return saveDeliveryStatus;
  }

  public void setSaveDeliveryStatus(boolean saveDeliveryStatus) {
    this.saveDeliveryStatus = saveDeliveryStatus;
  }

  public interface Receiver {
    public void execute(CalendarCheckMessageStatusCmd cmd);
  }
}
