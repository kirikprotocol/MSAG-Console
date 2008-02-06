package com.eyeline.sponsored.ds.subscription;

import com.eyeline.sponsored.ds.Storable;

import java.util.Date;

/**
 *
 * @author artem
 */
public interface Subscription extends Storable {

  public String getSubscriberAddress();

  public void setSubscriberAddress(String address);

  public String getDistributionName();

  public void setDistributionName(String name);

  public Date getStartDate();

  public void setStartDate(Date date);

  public Date getEndDate();

  public void setEndDate(Date date);

  public int getVolume();

  public void setVolume(int volume);
}
