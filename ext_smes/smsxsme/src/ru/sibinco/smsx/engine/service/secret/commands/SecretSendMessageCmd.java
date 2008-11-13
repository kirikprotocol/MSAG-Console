package ru.sibinco.smsx.engine.service.secret.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;

/**
 * User: artem
 * Date: Sep 11, 2007
 */

public class SecretSendMessageCmd extends Command {
  public static final int ERR_SOURCE_ABONENT_NOT_REGISTERED = ERR_INT + 1;
  public static final int ERR_DESTINATION_ADDRESS_IS_NOT_ALLOWED = ERR_INT + 3;

  private String sourceAddress;
  private String destinationAddress;
  private String message;
  private int destAddressSubunit;
  private boolean saveDeliveryStatus;
  private boolean notifyOriginator;
  private String mscAddress;
  private boolean appendAdverising;


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

  public String getMscAddress() {
    return mscAddress;
  }

  public void setMscAddress(String mscAddress) {
    this.mscAddress = mscAddress;
  }

  public boolean isNotifyOriginator() {
    return notifyOriginator;
  }

  public void setNotifyOriginator(boolean notifyOriginator) {
    this.notifyOriginator = notifyOriginator;
  }

  public boolean isAppendAdverising() {
    return appendAdverising;
  }

  public void setAppendAdverising(boolean appendAdverising) {
    this.appendAdverising = appendAdverising;
  }

  public interface Receiver {
    public long execute(SecretSendMessageCmd cmd) throws CommandExecutionException;
  }
}
