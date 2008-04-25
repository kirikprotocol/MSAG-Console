package com.eyeline.sponsored.ds.distribution.advert;

import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;

/**
 * User: artem
 * Date: 25.04.2008
 */

public interface DeliveryStatsQuery {
  public boolean isAllowed(DeliveryStat stat);
}
