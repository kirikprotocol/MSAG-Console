package ru.sibinco.smsx.engine.service.calendar.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

import java.util.Date;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class CalendarSendMessageCmd extends Command {

  public static final int ERR_WRONG_SEND_DATE = ERR_INT + 1;
  public static final int ERR_WRONG_DESTINATION_ADDRESS = ERR_INT + 2;

  // fields
  private String sourceAddress;
  private String destinationAddress;
  private String message;
  private Date sendDate;
  private int destAddressSubunit;
  private boolean storeDeliveryStatus;

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

  public Date getSendDate() {
    return sendDate;
  }

  public void setSendDate(Date sendDate) {
    this.sendDate = sendDate;
  }

  public int getDestAddressSubunit() {
    return destAddressSubunit;
  }

  public void setDestAddressSubunit(int destAddressSubunit) {
    this.destAddressSubunit = destAddressSubunit;
  }

  public boolean isStoreDeliveryStatus() {
    return storeDeliveryStatus;
  }

  public void setStoreDeliveryStatus(boolean storeDeliveryStatus) {
    this.storeDeliveryStatus = storeDeliveryStatus;
  }

  public interface Receiver {
    public long execute(CalendarSendMessageCmd cmd) throws CommandExecutionException;
  }
}
