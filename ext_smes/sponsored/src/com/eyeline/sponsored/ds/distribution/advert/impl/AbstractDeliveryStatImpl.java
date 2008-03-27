package com.eyeline.sponsored.ds.distribution.advert.impl;

import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;

/**
 *
 * @author artem
 */
public abstract class AbstractDeliveryStatImpl implements DeliveryStat {

  private String subscriberAddress;
  private int delivered;

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

  public int hashCode() {
    return subscriberAddress.hashCode();
  }

  public boolean equals(Object o) {
    if (!(o instanceof AbstractDeliveryStatImpl))
      return false;
    return subscriberAddress.equals(((AbstractDeliveryStatImpl)o).subscriberAddress);
  }

}
