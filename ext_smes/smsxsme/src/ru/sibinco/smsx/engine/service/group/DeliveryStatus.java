package ru.sibinco.smsx.engine.service.group;

/**
 * User: artem
 * Date: 24.02.2009
 */
public class DeliveryStatus {

  public static final byte ACCEPTED = 0;
  public static final byte SENT = 1;
  public static final byte DELIVERED = 2;
  public static final byte SYSTEM_ERROR = -1;
  public static final byte NOT_DELIVERED = -2;

  int status = ACCEPTED;
  final String address;

  public DeliveryStatus(String address) {
    this.address = address;
  }

  public String getAddress() {
    return address;
  }

  public int getStatus() {
    return status;
  }
}
