package com.eyeline.sponsored.ds.distribution.advert.impl;

import com.eyeline.sponsored.ds.distribution.advert.Delivery;

import java.util.Date;
import java.util.TimeZone;

/**
 *
 * @author artem
 */
public abstract class AbstractDeliveryImpl implements Delivery {
  private String subscriberAddress;
  private String distributionName;
  private Date startDate;
  private Date sendDate;
  private Date endDate;
  private int sended;
  private int total;
  private TimeZone timezone;

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

  public int getSended() {
    return sended;
  }

  public void setSended(int sended) {
    this.sended = sended;
  }

  public Date getStartDate() {
    return startDate;
  }

  public void setStartDate(Date startDate) {
    this.startDate = startDate;
  }

  public Date getSendDate() {
    return sendDate;
  }

  public void setSendDate(Date sendDate) {
    this.sendDate = sendDate;
  }

  public String getSubscriberAddress() {
    return subscriberAddress;
  }

  public void setSubscriberAddress(String subscriberAddress) {
    this.subscriberAddress = subscriberAddress;
  }

  public int getTotal() {
    return total;
  }

  public void setTotal(int total) {
    this.total = total;
  }

  public TimeZone getTimezone() {
    return timezone;
  }

  public void setTimezone(TimeZone timezone) {
    this.timezone = timezone;
  }
}
