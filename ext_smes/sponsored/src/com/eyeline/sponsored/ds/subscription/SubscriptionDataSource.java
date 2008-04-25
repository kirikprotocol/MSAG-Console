package com.eyeline.sponsored.ds.subscription;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.ResultSet;

import java.util.Date;
import java.util.TimeZone;

/**
 *
 * @author artem
 */
public interface SubscriptionDataSource {

  public DataSourceTransaction createTransaction() throws DataSourceException;

  /**
   * Creates new instance of Subscriber without saving
   * @return   
   */
  public Subscriber createSubscriber();

  /**
   * Creates new instance of Distribution without saving
   * @return
   */
  public Distribution createDistribution();

  /**
   * Creates new instance of subscription without saving
   * @return
   */
  public Subscription createSubscription();
  
  /**
   * Lookup subscriber by it's address.
   * @param subscriberAddress subscriber address
   * @return subscriber with specified address or nul if subscriber does not exists
   * @throws com.eyeline.utils.ds.DataSourceException
   */
  public Subscriber lookupSubscriber(String subscriberAddress) throws DataSourceException;
  
  /**
   * Lookup subscriber by it's address in specified transaction
   * @param subscriberAddress
   * @param tx
   * @return
   * @throws com.eyeline.sponsored.ds.DataSourceException
   */
  public Subscriber lookupSubscriber(String subscriberAddress, DataSourceTransaction tx) throws DataSourceException;

  /**
   * Lookup  distribution by it's name
   * @param distributionName distribution name
   * @return distribution or null if distribution with such name does not exists
   * @throws com.eyeline.utils.ds.DataSourceException
   */
  public Distribution lookupDistribution(String distributionName) throws DataSourceException;
  
  /**
   * Lookup  distribution by it's name in specified transaction
   * @param distributionName
   * @param tx
   * @return
   * @throws com.eyeline.sponsored.ds.DataSourceException
   */
  public Distribution lookupDistribution(String distributionName, DataSourceTransaction tx) throws DataSourceException;

  /**
   * Lookup subscription by subscriber address and distribution name
   * @param subscriberAddress subscriber address
   * @param distributionName distribution name
   * @return subscription or null if subscription does not exists
   * @throws com.eyeline.utils.ds.DataSourceException
   */
  public Subscription lookupActiveSubscription(String subscriberAddress, String distributionName, Date date) throws DataSourceException;
  
  /**
   * Lookup subscription by subscriber address and distribution name
   * @param subscriberAddress
   * @param distributionName
   * @param date
   * @param tx
   * @return
   * @throws com.eyeline.sponsored.ds.DataSourceException
   */
  public Subscription lookupActiveSubscription(String subscriberAddress, String distributionName, Date date, DataSourceTransaction tx) throws DataSourceException;        
  
  /**
   * Lookup full subscription info (distribution|subscription|subscriber) by distribution name, date and timezone
   * @param distributionName
   * @param date
   * @param timezone
   * @param tx
   * @return
   * @throws com.eyeline.sponsored.ds.DataSourceException
   */
  public ResultSet<SubscriptionRow> lookupFullSubscriptionInfo(String distributionName, Date date, TimeZone timezone, DataSourceTransaction tx) throws DataSourceException;

  /**
   *
   * @param distributionName
   * @param date
   * @param timezone
   * @param tx
   * @return
   * @throws DataSourceException
   */
  public ResultSet<VolumeStat> getVolumeStats(String distributionName, Date date, TimeZone timezone, DataSourceTransaction tx) throws DataSourceException;
}
