package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.util.Address;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class ArchiveMessage {

  private long id;
  private int deliveryId;
  private String deliveryName;
  private Address abonent; 
  private Date date;
  private String owner;
  private String text; 
  private MessageState state;
  private Integer errorCode;

  public long getId() {
    return id;
  }

  void setId(long id) {
    this.id = id;
  }

  public int getDeliveryId() {
    return deliveryId;
  }

  void setDeliveryId(int deliveryId) {
    this.deliveryId = deliveryId;
  }

  public String getDeliveryName() {
    return deliveryName;
  }

  void setDeliveryName(String deliveryName) {
    this.deliveryName = deliveryName;
  }

  public Address getAbonent() {
    return abonent;
  }

  void setAbonent(Address abonent) {
    this.abonent = abonent;
  }

  public Date getDate() {
    return date;
  }

  void setDate(Date date) {
    this.date = date;
  }

  public String getOwner() {
    return owner;
  }

  void setOwner(String owner) {
    this.owner = owner;
  }

  public String getText() {
    return text;
  }

  void setText(String text) {
    this.text = text;
  }

  public MessageState getState() {
    return state;
  }

  void setState(MessageState state) {
    this.state = state;
  }

  public Integer getErrorCode() {
    return errorCode;
  }

  void setErrorCode(Integer errorCode) {
    this.errorCode = errorCode;
  }
}
