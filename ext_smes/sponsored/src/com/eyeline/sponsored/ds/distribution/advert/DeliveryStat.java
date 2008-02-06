package com.eyeline.sponsored.ds.distribution.advert;

import com.eyeline.sponsored.ds.*;
import java.util.Date;

/**
 *
 * @author artem
 */
public interface DeliveryStat {
  
  public String getSubscriberAddress();
  public void setSubscriberAddress(String address);    
  
  public int getDelivered();
  public void setDelivered(int delivered);
}
