package mobi.eyeline.informer.web.controllers.stats;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 19.11.2010
 * Time: 16:35:35
 */
public interface DeliveryStatTotals {
  public void reset();
  public void add(AggregatedRecord r);
}
