package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.util.Address;

import java.util.Date;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 18:34:34
 */
public class DeliveryMessageNotification extends DeliveryNotification {
  private long messageId;
  private MessageState messageState;
  private int smppStatus;
  private Address address;
  private String userData;

  public DeliveryMessageNotification(DeliveryNotificationType type, Date eventDate, int deliveryId, String userId, long messageId, MessageState messageState, int smppStatus, Address address, String userData) {
    super(type, eventDate, deliveryId, userId);
    this.messageId = messageId;
    this.messageState = messageState;
    this.smppStatus = smppStatus;
    this.address = address;
    this.userData = userData;
  }

  public long getMessageId() {
    return messageId;
  }

  public MessageState getMessageState() {
    return messageState;
  }

  public int getSmppStatus() {
    return smppStatus;
  }

  public Address getAddress() {
    return address;
  }

  public String getUserData() {
    return userData;
  }
}
