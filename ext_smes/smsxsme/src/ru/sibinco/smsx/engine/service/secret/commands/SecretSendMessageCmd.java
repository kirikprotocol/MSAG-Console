package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SecretSendMessageCmd extends Command {
  public static final int STATUS_SOURCE_ABONENT_NOT_REGISTERED = STATUS_INT + 1;
  public static final int STATUS_DESTINATION_ABONENT_NOT_REGISTERED = STATUS_INT + 2;
  public static final int STATUS_DESTINATION_ADDRESS_IS_NOT_ALLOWED = STATUS_INT + 3;

  private String sourceAddress;
  private String destinationAddress;
  private String message;
  private int destAddressSubunit;
  private boolean saveDeliveryStatus;
  private boolean notifyOriginator;
  private long msgId;

  public String getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public String getDestinationAddress() {
    return destinationAddress;
  }

  public void setDestinationAddress(String destinationAddress) {
    this.destinationAddress = destinationAddress;
  }

  public String getMessage() {
    return message;
  }

  public void setMessage(String message) {
    this.message = message;
  }

  public int getDestAddressSubunit() {
    return destAddressSubunit;
  }

  public void setDestAddressSubunit(int destAddressSubunit) {
    this.destAddressSubunit = destAddressSubunit;
  }

  public boolean isSaveDeliveryStatus() {
    return saveDeliveryStatus;
  }

  public void setSaveDeliveryStatus(boolean saveDeliveryStatus) {
    this.saveDeliveryStatus = saveDeliveryStatus;
  }

  public boolean isNotifyOriginator() {
    return notifyOriginator;
  }

  public void setNotifyOriginator(boolean notifyOriginator) {
    this.notifyOriginator = notifyOriginator;
  }

  public long getMsgId() {
    return msgId;
  }

  public void setMsgId(long msgId) {
    this.msgId = msgId;
  }

  public interface Receiver {
    public void execute(SecretSendMessageCmd cmd);
  }
}
