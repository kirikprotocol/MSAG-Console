package mobi.eyeline.informer.admin.delivery.changelog;

import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.util.Address;

import java.util.Date;
import java.util.Properties;

/**
 * Нотификация об изменении статуса сообщения
 *
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 18:34:34
 */
public class ChangeMessageStateEvent  {
  private final Date eventDate;
  private final int deliveryId;
  private final String userId;
  private final long messageId;
  private final MessageState messageState;
  private final int smppStatus;
  private final Address address;
  private final Properties properties;

  public ChangeMessageStateEvent(Date eventDate, int deliveryId, String userId, long messageId, MessageState messageState, int smppStatus, Address address, Properties properties) {
    this.eventDate = eventDate;
    this.deliveryId = deliveryId;
    this.userId = userId;
    this.messageId = messageId;
    this.messageState = messageState;
    this.smppStatus = smppStatus;
    this.address = address;
    this.properties = properties;
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

  public Properties getProperties() {
    return properties;
  }

  public Date getEventDate() {
    return eventDate;
  }

  public int getDeliveryId() {
    return deliveryId;
  }

  public String getUserId() {
    return userId;
  }
}
