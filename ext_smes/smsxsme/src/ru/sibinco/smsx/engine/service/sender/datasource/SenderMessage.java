package ru.sibinco.smsx.engine.service.sender.datasource;

/**
 * User: artem
 * Date: 06.07.2007
 */

public class SenderMessage {

  public static final int STATUS_NEW = 0;
  public static final int STATUS_PROCESSED = 1;
  public static final int STATUS_DELIVERED = 2;
  public static final int STATUS_DELIVERY_FAILED = 3;

  private int id = -1;
  private int status = STATUS_NEW;
  private int smppStatus = -1;
  private String sourceAddress;
  private String destinationAddress;
  private String message;
  private int destAddrSubunit;
  private boolean storable = true;
  private String connectionName;

  public SenderMessage() {
  }

  public SenderMessage(int id) {
    setId(id);
  }

  public int getId() {
    return id;
  }

  void setId(int id) {
    this.id = id;
  }

  public int getSmppStatus() {
    return smppStatus;
  }

  public void setSmppStatus(int smppStatus) {
    this.smppStatus = smppStatus;
  }

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

  public int getDestAddrSubunit() {
    return destAddrSubunit;
  }

  public void setDestAddrSubunit(int destAddrSubunit) {
    this.destAddrSubunit = destAddrSubunit;
  }

  public boolean isStorable() {
    return storable;
  }

  public void setStorable(boolean storable) {
    this.storable = storable;
  }

  public int getStatus() {
    return status;
  }

  public void setStatus(int status) {
    this.status = status;
  }

  public String getConnectionName() {
    return connectionName;
  }

  public void setConnectionName(String connectionName) {
    this.connectionName = connectionName;
  }

  public boolean isExists() {
    return id != -1;
  }

}
