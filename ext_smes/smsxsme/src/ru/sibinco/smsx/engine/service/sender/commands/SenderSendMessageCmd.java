package ru.sibinco.smsx.engine.service.sender.commands;

import ru.sibinco.smsx.engine.service.AsyncCommand;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SenderSendMessageCmd extends AsyncCommand {

  private String sourceAddress;
  private String destinationAddress;
  private String message;
  private int destAddressSubunit;
  private boolean storable;

  private int msgId;

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

  public boolean isStorable() {
    return storable;
  }

  public void setStorable(boolean storable) {
    this.storable = storable;
  }

  public int getMsgId() {
    return msgId;
  }

  public void setMsgId(int msgId) {
    this.msgId = msgId;
  }

  public interface Receiver {
    public void execute(SenderSendMessageCmd cmd);
  }

}
