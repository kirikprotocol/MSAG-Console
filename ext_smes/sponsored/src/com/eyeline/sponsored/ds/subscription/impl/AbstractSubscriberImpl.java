package com.eyeline.sponsored.ds.subscription.impl;

import com.eyeline.sponsored.ds.subscription.Subscriber;
import java.util.TimeZone;

/**
 *
 * @author artem
 */
public abstract class AbstractSubscriberImpl implements Subscriber {

  private String address;
  private TimeZone timeZone;

  public String getAddress() {
    return address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public TimeZone getTimeZone() {
    return timeZone;
  }

  public void setTimeZone(TimeZone timeZone) {
    this.timeZone = timeZone;
  }
}
