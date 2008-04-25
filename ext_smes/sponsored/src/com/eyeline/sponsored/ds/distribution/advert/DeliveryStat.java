package com.eyeline.sponsored.ds.distribution.advert;

/**
 *
 * @author artem
 */
public interface DeliveryStat {

  public String getSubscriberAddress();
  public void setSubscriberAddress(String address);

  public int getDelivered();
  public void setDelivered(int delivered);

  public int getSended();
  public void setSended(int sended);

  public int getAdvertiserId();
  public void setAdvertiserId(int id);
}
