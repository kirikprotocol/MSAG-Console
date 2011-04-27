package ru.sibinco.sponsored.stats.backend.datasource;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryStat {   //todo сделать package local конструктор

  private String subscriberAddress;
  private int delivered = 0;
  private int advertiserId = -1;
  private int sended = 0;

  public String getSubscriberAddress() {
    return subscriberAddress;
  }

  public void setSubscriberAddress(String subscriberAddress) {
    this.subscriberAddress = subscriberAddress;
  }

  public int getDelivered() {
    return delivered;
  }

  public void setDelivered(int delivered) {
    this.delivered = delivered;
  }

  public int getAdvertiserId() {
    return advertiserId;
  }

  public void setAdvertiserId(int advertiserId) {
    this.advertiserId = advertiserId;
  }

  public int getSended() {
    return sended;
  }

  public void setSended(int sended) {
    this.sended = sended;
  }

  public int hashCode() {
    return subscriberAddress.hashCode() + 31*advertiserId;
  }

  public boolean equals(Object o) {
    if (!(o instanceof DeliveryStat))
      return false;
    DeliveryStat o1 = ((DeliveryStat)o);
    return subscriberAddress.equals(o1.subscriberAddress) && advertiserId == o1.advertiserId;
  }
}
