package com.eyeline.sponsored.ds.subscription.impl;

import com.eyeline.sponsored.ds.subscription.Distribution;
import com.eyeline.sponsored.ds.subscription.Subscriber;
import com.eyeline.sponsored.ds.subscription.Subscription;
import com.eyeline.sponsored.ds.subscription.SubscriptionRow;

/**
 *
 * @author artem
 */
public class AbstractSubscriptionRowImpl implements SubscriptionRow {

  private Subscriber subscriber;
  private Subscription subscription;
  private Distribution distribution;

  public Subscriber getSubscriber() {
    return subscriber;
  }

  public Subscription getSubscription() {
    return subscription;
  }

  public Distribution getDistribution() {
    return distribution;
  }

  public void setDistribution(Distribution distribution) {
    this.distribution = distribution;
  }

  public void setSubscriber(Subscriber subscriber) {
    this.subscriber = subscriber;
  }

  public void setSubscription(Subscription subscription) {
    this.subscription = subscription;
  }
}
