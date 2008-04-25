package com.eyeline.sponsored.distribution.advert.distr.core;

import com.eyeline.sponsored.ds.distribution.advert.Delivery;

import java.util.concurrent.ArrayBlockingQueue;

/**
 * User: artem
 * Date: 22.04.2008
 */

class DeliveriesQueue extends ArrayBlockingQueue<Delivery> {
  private DeliveryModificator modificator = null;

  public DeliveriesQueue() {
    super(100000);
  }

  public void setModificator(DeliveryModificator modificator) {
    this.modificator = modificator;
  }

  public boolean add(Delivery d) {
    if (modificator != null)
      modificator.modifyDelivery(d);
    return super.add(d);
  }
}
