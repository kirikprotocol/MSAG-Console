package ru.sibinco.smsx.engine.service.calendar.datasource;

import java.sql.Timestamp;

/**
 * User: artem
 * Date: Jul 31, 2006
 */

public class CalendarMessage  {

  public static final int STATUS_NEW = 0;
  public static final int STATUS_PROCESSED = 1;
  public static final int STATUS_DELIVERED = 2;
  public static final int STATUS_DELIVERY_FAILED = 3;

  private int id;
  private String sourceAddress;
  private String destinationAddress;
  private Timestamp sendDate;
  private int destAddressSubunit;
  private String message;
  private int status = STATUS_NEW;
  private int smppStatus = -1;
  private boolean saveDeliveryStatus;
  private String connectionName;
  private long smppId;


  CalendarMessage(final int id) {
    this.id = id;
  }

  public CalendarMessage() {
    this(-1);
  }

  void setId(int id) {
    this.id = id;
  }

  public int getId() {
    return id;
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

  public Timestamp getSendDate() {
    return sendDate;
  }

  public void setSendDate(Timestamp sendDate) {
    this.sendDate = sendDate;
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
