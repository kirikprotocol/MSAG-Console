package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.delivery.DeliveryStatRecord;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 19.11.2010
 * Time: 16:34:02
 */
public class DeliveriesCountByPeriodTotals implements DeliveryStatTotals{
  int counter;

  public DeliveriesCountByPeriodTotals() {
    reset();
  }

  public void reset() {
     counter=0;
  }

  public void add(AggregatedRecord r) {
    DeliveriesCountByPeriodRecord dcr = (DeliveriesCountByPeriodRecord) r;
    this.counter+=((DeliveriesCountByPeriodRecord) r).getCounter();
  }

  public int getCounter() {
    return counter;
  }
}
