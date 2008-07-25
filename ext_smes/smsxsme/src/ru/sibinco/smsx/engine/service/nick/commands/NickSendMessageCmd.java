package ru.sibinco.smsx.engine.service.nick.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class NickSendMessageCmd extends AsyncCommand {

  public static final int STATUS_INVALID_SOURCE_ADDRESS = STATUS_INT + 1;
  public static final int STATUS_INVALID_DESTINATION_ADDRESS = STATUS_INT + 2;
  public static final int STATUS_DELIVERED = STATUS_INT + 3;
  public static final int STATUS_MESSAGE_SENDED = STATUS_INT + 4;

  private String sourceAddress;
  private String destinationAddress;
  private String message;
  private String mscAddress;
  private String imsi;
  private int smppStatus;

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

  public String getMscAddress() {
    return mscAddress;
  }

  public void setMscAddress(String mscAddress) {
    this.mscAddress = mscAddress;
  }

  public String getImsi() {
    return imsi;
  }

  public void setImsi(String imsi) {
    this.imsi = imsi;
  }

  public int getSmppStatus() {
    return smppStatus;
  }

  public void setSmppStatus(int smppStatus) {
    this.smppStatus = smppStatus;
  }

  public interface Receiver {
    public void execute(NickSendMessageCmd cmd);
  }
}
