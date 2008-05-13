package ru.sibinco.smsx.engine.service.secret.datasource;

import java.sql.Timestamp;

/**
 * User: artem
 * Date: 29.06.2007
 */

public class SecretMessage {

  public static final int STATUS_NEW = 0;
  public static final int STATUS_PROCESSED = 1;
  public static final int STATUS_DELIVERED = 2;
  public static final int STATUS_DELIVERY_FAILED = 3;

  private int id = -1;
  private String destinationAddress;
  private String message;
  private String sourceAddress;
  private int destAddressSubunit;
  private Timestamp sendDate;
  private int status;
  private int smppStatus = -1;
  private boolean saveDeliveryStatus;
  private boolean notifyOriginator;
  private String connectionName;
  private long smppId;

  public SecretMessage() {
    this(-1);
  }

  public SecretMessage(final int id) {
    this.id = id;
  }

  public int getId() {
    return id;
  }

  void setId(int id) {
    this.id = id;
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

  public String getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public int getDestAddressSubunit() {
    return destAddressSubunit;
  }

  public void setDestAddressSubunit(int destAddressSubunit) {
    this.destAddressSubunit = destAddressSubunit;
  }

  public Timestamp getSendDate() {
    return sendDate;
  }

  public void setSendDate(Timestamp sendDate) {
    this.sendDate = sendDate;
  }

  public int getStatus() {
    return status;
  }

  public void setStatus(int status) {
    this.status = status;
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

  public boolean isNotifyOriginator() {
    return notifyOriginator;
  }

  public void setNotifyOriginator(boolean notifyOriginator) {
    this.notifyOriginator = notifyOriginator;
  }

  public String getConnectionName() {
    return connectionName;
  }

  public void setConnectionName(String connectionName) {
    this.connectionName = connectionName;
  }

  public long getSmppId() {
    return smppId;
  }

  public void setSmppId(long smppId) {
    this.smppId = smppId;
  }

  public boolean isExists() {
    return id != -1;
  }
}
