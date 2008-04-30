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

  public Subscriber createSubscriber();

  public Distribution createDistribution();

  public Subscription createSubscription();

  public Subscriber lookupSubscriber(String subscriberAddress) throws DataSourceException;
  
  public Subscriber lookupSubscriber(String subscriberAddress, DataSourceTransaction tx) throws DataSourceException;

  public Distribution lookupDistribution(String distributionName) throws DataSourceException;
  
  public Distribution lookupDistribution(String distributionName, DataSourceTransaction tx) throws DataSourceException;

  public Subscription lookupActiveSubscription(String subscriberAddress, String distributionName, Date date) throws DataSourceException;
  
  public Subscription lookupActiveSubscription(String subscriberAddress, String distributionName, Date date, DataSourceTransaction tx) throws DataSourceException;

  public ResultSet<Subscription> lookupActiveSubscriptions(String distributionName, Date date, DataSourceTransaction tx) throws DataSourceException;

  public ResultSet<SubscriptionRow> lookupFullSubscriptionInfo(String distributionName, Date date, TimeZone timezone, DataSourceTransaction tx) throws DataSourceException;

  public ResultSet<VolumeStat> getVolumeStats(String distributionName, Date date, TimeZone timezone, DataSourceTransaction tx) throws DataSourceException;
}
