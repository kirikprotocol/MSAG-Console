package com.eyeline.sponsored.distribution.advert.distr.core;

import com.eyeline.sponsored.utils.CalendarUtils;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.banner.BannerMap;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsDataSource;
import ru.sibinco.smsc.utils.timezones.SmscTimezone;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesListException;

import java.util.Date;
import java.util.TimeZone;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class DeliveryStatsProcessor {

  private static DeliveryStatsProcessor instance;

  private final DeliveryStatsDataSource ds;
  private final SmscTimezonesList timezones;
  private final BannerMap bannerMap;

  public DeliveryStatsProcessor(DeliveryStatsDataSource ds, SmscTimezonesList timezones, BannerMap map) {
    this.ds = ds;
    this.timezones = timezones;
    this.bannerMap = map;
  }

  public static void init(DeliveryStatsDataSource ds, SmscTimezonesList timezones, BannerMap map) {
    instance = new DeliveryStatsProcessor(ds, timezones, map);
  }

  public static DeliveryStatsProcessor getInstance() {
    return instance;
  }

  public void registerDelivery(String subscriberAddress, String messageId, int deliveryInc) throws ProcessorException {

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

    // Get advertiser id from banner map
    int advertiserId = -1;
    try {
      advertiserId = bannerMap.get(Long.parseLong(messageId));
    } catch (NumberFormatException e) {
      throw new ProcessorException(e);
    }

    // Update delivery stats
    try {
      ds.addDeliveryStat(subscriberAddress, advertiserId, tzDayStart, deliveryInc, 0);
    } catch (DataSourceException e) {
      throw new ProcessorException(e);
    }

  }

}
