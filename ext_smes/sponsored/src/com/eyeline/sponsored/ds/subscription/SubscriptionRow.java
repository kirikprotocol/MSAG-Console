package com.eyeline.sponsored.ds.subscription;

/**
 *
 * @author artem
 */
public interface SubscriptionRow {

  public Subscriber getSubscriber();

  public Subscription getSubscription();

  public Distribution getDistribution();
}
