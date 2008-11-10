package com.eyeline.sponsored.subscription.service.core;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.subscription.Distribution;
import com.eyeline.sponsored.ds.subscription.Subscriber;
import com.eyeline.sponsored.ds.subscription.Subscription;
import com.eyeline.sponsored.ds.subscription.SubscriptionDataSource;
import org.apache.log4j.Category;
import ru.sibinco.smsc.utils.timezones.SmscTimezone;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesListException;

import java.util.Date;
import java.util.TimeZone;

/**
 *
 * @author artem
 */
public class SubscriptionProcessor {

  private static final Category log = Category.getInstance("SUBSCRIPTION");
  private static SubscriptionProcessor instance = null;
  //
  private final SubscriptionDataSource ds;
  private final SmscTimezonesList timezones;

  public static void init(SubscriptionDataSource ds, SmscTimezonesList timezones) {
    instance = new SubscriptionProcessor(ds, timezones);
  }

  public static SubscriptionProcessor getInstance() {
    return instance;
  }

  public SubscriptionProcessor(SubscriptionDataSource ds, SmscTimezonesList timezones) {
    this.ds = ds;
    this.timezones = timezones;
  }

  private Subscriber createSubscriber(String subscriberAddress, DataSourceTransaction t) throws ProcessorException {
    final Subscriber subscriber = ds.createSubscriber();
    subscriber.setAddress(subscriberAddress);
    SmscTimezone tz;
    try {
      tz = timezones.getTimezoneByAddress(subscriberAddress);
    } catch (SmscTimezonesListException e) {
      log.error("Could't lookup timezone: subscr=" + subscriberAddress, e);
      throw new ProcessorException("Couldn't lookup timezone: subscr=" + subscriberAddress, e);
    }
    if (tz == null) {
      throw new ProcessorException("Couldn't find timezone for subscr=" + subscriberAddress);
    }

    subscriber.setTimeZone(TimeZone.getTimeZone(tz.getName()));

    try {
      subscriber.save(t);
    } catch (DataSourceException e) {
      log.error("Couldn't save subscriber=" + subscriberAddress, e);
      throw new ProcessorException("Couldn't save subscriber=" + subscriberAddress, e);
    }
    return subscriber;
  }

  private Subscription createSubscription(String subscriberAddress, String distributionName, int volume, DataSourceTransaction t) throws ProcessorException {
    final Subscription subscription = ds.createSubscription();
    subscription.setSubscriberAddress(subscriberAddress);
    subscription.setDistributionName(distributionName);
    subscription.setStartDate(new Date());
    subscription.setVolume(volume);

    try {
      subscription.save(t);
    } catch (DataSourceException e) {
      log.error("Could't save subscription: subscr=" + subscriberAddress + "; distr=" + distributionName);
      throw new ProcessorException("Couldn't save subscription: subscr=" + subscriberAddress + "; distr=" + distributionName, e);
    }

    return subscription;
  }

  private Subscriber getSubscriber(String subscriberAddress, DataSourceTransaction t) throws ProcessorException {
    // Lookup subscriber    
    try {
      return ds.lookupSubscriber(subscriberAddress, t);
    } catch (DataSourceException e) {
      log.error("Could't lookup subscriber: subscr=" + subscriberAddress);
      throw new ProcessorException("Couldn't lookup subscriber: subscr=" + subscriberAddress, e);
    }
  }

  private Distribution getDistribution(String distributionName, DataSourceTransaction t) throws ProcessorException {
    // Lookup distribution
    Distribution distribution;
    try {
      distribution = ds.lookupDistribution(distributionName, t);
    } catch (DataSourceException e) {
      log.error("Couldn't lookup distribution=" + distributionName, e);
      throw new ProcessorException("Couldn't lookup distribution=" + distributionName, e);
    }
    return distribution;
  }

  private Subscription getActiveSubscription(String subscriberAddress, String distributionName, DataSourceTransaction t) throws ProcessorException {
    try {
      return ds.lookupActiveSubscription(subscriberAddress, distributionName, new Date(), t);
    } catch (DataSourceException e) {
      log.error("Could't lookup subscription: subscr=" + subscriberAddress + "; distr=" + distributionName);
      throw new ProcessorException("Couldn't lookup subscription: subscr=" + subscriberAddress + "; distr=" + distributionName, e);
    }
  }

  public boolean isSubscribed(String subscriberAddress, String distributionName) throws ProcessorException {
    DataSourceTransaction t = null;
    try {
      t = ds.createTransaction();

      // Lookup active subscription on this distribution
      return getActiveSubscription(subscriberAddress, distributionName, t) != null;
    } catch (Throwable e) {
      if (t != null) {
        try {
          t.rollback();
        } catch (DataSourceException ex) {
          log.error("Couldn't roll back transaction", ex);
        }
      }
      throw new ProcessorException("Subscription failed", e);
    } finally {
      if (t != null) {
        t.close();
      }
    }
  }

  /**
   * Subscribe specified subscriber on specified distribution
   * @param subscriberAddress
   * @param distributionName
   * @return true, if subscription opened and false otherwise
   */
  public SubscriptionResult subscribe(String subscriberAddress, String distributionName, int volume) throws ProcessorException {
    DataSourceTransaction t = null;
    try {
      t = ds.createTransaction();

      // Lookup active subscription on this distribution
      final Subscription activeSubscription = getActiveSubscription(subscriberAddress, distributionName, t);

      if (activeSubscription == null) {
        // Lookup distribution
        final Distribution distribution = getDistribution(distributionName, t);
        if (distribution == null) {
          throw new ProcessorException("Distribution=" + distributionName + " not found");
        }

        // If active subscription not found check distribution opened
        if (distribution.getStatus() == Distribution.Status.CLOSED) {
          return SubscriptionResult.SUBSCRIPTION_CLOSED;
        }

        // Create subscriber if not exests
        if (ds.lookupSubscriber(subscriberAddress, t) == null) {          
          createSubscriber(subscriberAddress, t);
        }

        // Create subscription    
        createSubscription(subscriberAddress, distributionName, volume, t);

      } else {
        if (activeSubscription.getVolume() != volume) {
          unsubscribe(subscriberAddress, distributionName, t);
          createSubscription(subscriberAddress, distributionName, volume, t);
        }
      }

      t.commit();

      return SubscriptionResult.SUCCESS;
    } catch (Throwable e) {
      if (t != null) {
        try {
          t.rollback();
        } catch (DataSourceException ex) {
          log.error("Couldn't roll back transaction", ex);
        }
      }
      throw new ProcessorException("Subscription failed", e);
    } finally {
      if (t != null) {
        t.close();
      }
    }
  }

  /**
   * 
   * @param subscriberAddress
   * @param distributionName
   * @throws com.eyeline.sponsored.subscription.service.core.ProcessorException
   */
  public void unsubscribe(String subscriberAddress, String distributionName) throws ProcessorException {
    DataSourceTransaction t = null;
    try {
      t = ds.createTransaction();
      unsubscribe(subscriberAddress, distributionName, t);
      t.commit();
    } catch (Throwable e) {
      if (t != null) {
        try {
          t.rollback();
        } catch (DataSourceException ex) {
          log.error("Couldn't roll back transaction", ex);
        }
      }
      throw new ProcessorException("Unsubscription failed", e);
    } finally {
      if (t != null) {
        t.close();
      }
    }
  }

  /**
   * 
   * @param subscriberAddress
   * @param distributionName
   * @param t
   * @throws com.eyeline.sponsored.subscription.service.core.ProcessorException
   */
  private void unsubscribe(String subscriberAddress, String distributionName, DataSourceTransaction t) throws ProcessorException {
    final Subscription subscription = getActiveSubscription(subscriberAddress, distributionName, t);
    if (subscription != null) {
      subscription.setEndDate(new Date());
      try {
        subscription.save(t);
      } catch (DataSourceException e) {
        log.error("Couldn't update subscription: subscr=" + subscriberAddress + "; distr=" + distributionName, e);
        throw new ProcessorException("Couldn't update subscription: subscr=" + subscriberAddress + "; distr=" + distributionName);
      }
    }
  }

  public static enum SubscriptionResult {

    SUCCESS, SUBSCRIPTION_CLOSED
  }
}
