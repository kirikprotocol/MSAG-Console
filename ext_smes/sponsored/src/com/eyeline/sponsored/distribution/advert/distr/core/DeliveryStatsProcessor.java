package com.eyeline.sponsored.distribution.advert.distr.core;

import com.eyeline.sponsored.ds.distribution.advert.DistributionDataSource;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.distribution.advert.utils.CalendarUtils;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesListException;
import ru.sibinco.smsc.utils.timezones.SmscTimezone;

import java.util.TimeZone;
import java.util.Date;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class DeliveryStatsProcessor {

  private static DeliveryStatsProcessor instance;

  private final DistributionDataSource ds;
  private final SmscTimezonesList timezones;

  public DeliveryStatsProcessor(DistributionDataSource ds, SmscTimezonesList timezones) {
    this.ds = ds;
    this.timezones = timezones;
  }

  public static void init(DistributionDataSource ds, SmscTimezonesList timezones) {
    instance = new DeliveryStatsProcessor(ds, timezones);
  }

  public static DeliveryStatsProcessor getInstance() {
    return instance;
  }

  public synchronized void registerDelivery(String subscriberAddress, int deliveryInc) throws ProcessorException {

    // Calculate date according subscriber's time zone
    Date tzDayStart;
    try {
      final SmscTimezone subscriberTZ = timezones.getTimezoneByAddress(subscriberAddress);
      if (subscriberTZ == null) {
        throw new ProcessorException("Timezone not found for subscr=" + subscriberAddress);
      }

      final Date tzTime = CalendarUtils.convertFromLocalToTZ(new Date(), TimeZone.getTimeZone(subscriberTZ.getName()));

      tzDayStart = CalendarUtils.getDayStart(tzTime);

    } catch (SmscTimezonesListException e) {
      throw new ProcessorException(e);
    }

    // Update delivery stats
    try {
      ds.updateDeliveryStat(subscriberAddress, tzDayStart, deliveryInc);
    } catch (DataSourceException e) {
      throw new ProcessorException(e);
    }

  }

}
