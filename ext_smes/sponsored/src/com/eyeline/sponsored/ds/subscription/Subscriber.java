package com.eyeline.sponsored.ds.subscription;

import com.eyeline.sponsored.ds.Storable;

import java.util.TimeZone;

/**
 *
 * @author artem
 */
public interface Subscriber extends Storable {

  public String getAddress();

  public void setAddress(String address);

  public TimeZone getTimeZone();

  public void setTimeZone(TimeZone timeZone);
}
