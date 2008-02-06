package com.eyeline.sponsored.ds.distribution.advert.impl;

import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;
import java.util.Date;

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

}
