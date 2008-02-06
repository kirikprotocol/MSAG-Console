package com.eyeline.sponsored.ds.distribution.advert;

import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.Storable;
import com.eyeline.sponsored.ds.DataSourceException;

import java.util.Date;
import java.util.TimeZone;

/**
 *
 * @author artem
 */
public interface Delivery extends Storable {
  public String getDistributionName();
  public void setDistributionName(String name);
  
  public String getSubscriberAddress();
  public void setSubscriberAddress(String address);
  
  public Date getStartDate();
  public void setStartDate(Date startDate);

  public Date getSendDate();
  public void setSendDate(Date sendDate);
  
  public Date getEndDate();
  public void setEndDate(Date endDate);
  
  public int getSended();
  public void setSended(int sended);
  
  public int getTotal();
  public void setTotal(int total);

  public TimeZone getTimezone();
  public void setTimezone(TimeZone tz);

  public void update() throws DataSourceException;
}
