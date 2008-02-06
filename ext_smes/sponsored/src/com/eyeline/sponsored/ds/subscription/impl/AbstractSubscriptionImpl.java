package com.eyeline.sponsored.ds.subscription.impl;

import com.eyeline.sponsored.ds.subscription.Subscription;
import java.util.Date;

/**
 *
 * @author artem
 */
public abstract class AbstractSubscriptionImpl implements Subscription {

  private String subscriberAddress;
  private String distributionName;
  private int volume;
  private Date startDate;
  private Date endDate;

  public String getDistributionName() {
    return distributionName;
  }

  public void setDistributionName(String distributionName) {
    this.distributionName = distributionName;
  }

  public Date getEndDate() {
    return endDate;
  }

  public void setEndDate(Date endDate) {
    this.endDate = endDate;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) {
    this.startDate = startDate;
  }

  public String getSubscriberAddress() {
    return subscriberAddress;
  }

  public void setSubscriberAddress(String subscriberAddress) {
    this.subscriberAddress = subscriberAddress;
  }

  public int getVolume() {
    return volume;
  }

  public void setVolume(int volume) {
    this.volume = volume;
  }
}
