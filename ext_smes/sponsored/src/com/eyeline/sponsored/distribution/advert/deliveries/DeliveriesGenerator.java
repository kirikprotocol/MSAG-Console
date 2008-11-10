package com.eyeline.sponsored.distribution.advert.deliveries;

import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.sponsored.distribution.advert.config.DistributionInfo;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.ResultSet;
import com.eyeline.sponsored.ds.distribution.advert.DeliveriesDataSource;
import com.eyeline.sponsored.ds.distribution.advert.Delivery;
import com.eyeline.sponsored.ds.distribution.advert.impl.db.DBDistributionDataSource;
import com.eyeline.sponsored.ds.distribution.advert.impl.file.deliveries.FileDeliveriesDataSource;
import com.eyeline.sponsored.ds.subscription.SubscriptionDataSource;
import com.eyeline.sponsored.ds.subscription.SubscriptionRow;
import com.eyeline.sponsored.ds.subscription.VolumeStat;
import com.eyeline.sponsored.ds.subscription.impl.db.DBSubscriptionDataSource;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import ru.sibinco.smsc.utils.timezones.SmscTimezone;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;

import java.io.File;
import java.util.*;

/**
 * User: artem
 * Date: 03.02.2008
 */

public class DeliveriesGenerator {

  private final DeliveriesDataSource distributionDS;
  private final SubscriptionDataSource subscriptionDS;
  private final List<DistributionInfo> distrInfos;
  private final Set<TimeZone> timezones;

  public DeliveriesGenerator(DeliveriesDataSource distributionDS, SubscriptionDataSource subscriptionDS) {
    this.distributionDS = distributionDS;
    this.subscriptionDS = subscriptionDS;
    this.distrInfos = new LinkedList<DistributionInfo>();
    this.timezones = new HashSet<TimeZone>(50);
  }

  public void addDistribution(DistributionInfo info) {
    this.distrInfos.add(info);
  }

  public void addTimezone(TimeZone tz) {
    this.timezones.add(tz);
  }


  private Map<Integer, VolumeGroup> createVolumeGroups(String distributionName, Date startDate, Date endDate, TimeZone tz) throws DataSourceException {
    final double distrInterval = endDate.getTime() - startDate.getTime();

    DataSourceTransaction stx = null;

    // Create Volume groups
    final Map<Integer, VolumeGroup> volumeGroups = new HashMap<Integer, VolumeGroup>(50);
    ResultSet<VolumeStat> volumeStats = null;
    try {
      stx = subscriptionDS.createTransaction();
      volumeStats = subscriptionDS.getVolumeStats(distributionName, startDate, tz, stx);
      VolumeStat stat;
      VolumeGroup group;
      while (volumeStats.next()) {
        stat = volumeStats.get();
        group = new VolumeGroup();
        group.volume = stat.getVolume();
        group.numberOfSubscribers = stat.getNumberOfSubscribers();
        group.totalNumberOfMessages = stat.getVolume() * stat.getNumberOfSubscribers();
        group.delayBetweenMessages = distrInterval / group.totalNumberOfMessages;
        group.msgPerSecond = group.totalNumberOfMessages / (distrInterval / 1000);
        volumeGroups.put(group.volume, group);
        System.out.println("Volume group: vol=" + group.volume + "; subscr=" + group.numberOfSubscribers + "; msgs=" + group.totalNumberOfMessages + "; delay=" + group.delayBetweenMessages + "; sndSpeed=" + group.msgPerSecond);
      }

    } catch (DataSourceException e) {
      e.printStackTrace();
      throw new DataSourceException(e);
    } finally {
      try {
        if (volumeStats != null)
          volumeStats.close();
      } catch (DataSourceException e) {
        e.printStackTrace();
      }
      if (stx != null)
        stx.close();
    }

    return volumeGroups;
  }

  private void scheduleDeliveries(String distributionName, Date startDate, Date endDate, Map<Integer, VolumeGroup> volumeGroups, TimeZone tz) {
    DataSourceTransaction stx = null;
    ResultSet<SubscriptionRow> rs = null;

    // Create transactions
    final Map<Integer, DataSourceTransaction> dtxs = new HashMap<Integer, DataSourceTransaction>(10);

    try {
      // Create subscriptions data source transaction
      stx = subscriptionDS.createTransaction();

      // Lookup subscriptions that will be active at specified time
      rs = subscriptionDS.lookupFullSubscriptionInfo(distributionName, startDate, tz, stx);

      // Create distribution data source transaction
      for (VolumeGroup g : volumeGroups.values())
        dtxs.put(g.volume, distributionDS.createInsertTransaction(startDate, endDate, distributionName, g.volume, tz, (int)Math.round(g.numberOfSubscribers)));

      Delivery delivery;
      while(rs.next()) {
        SubscriptionRow row = rs.get();
        delivery = distributionDS.createDelivery();
        delivery.setDistributionName(row.getDistribution().getName());
        delivery.setSended(0);
        delivery.setSubscriberAddress(row.getSubscriber().getAddress());
        delivery.setTotal(row.getSubscription().getVolume());
        delivery.setTimezone(row.getSubscriber().getTimeZone());

        // Schedule delivery
        VolumeGroup g = volumeGroups.get(row.getSubscription().getVolume());
        delivery.setStartDate(new Date(startDate.getTime() + Math.round(g.abonentNumber * g.delayBetweenMessages)));
        delivery.setSendDate(delivery.getStartDate());
        delivery.setEndDate(new Date(endDate.getTime() - Math.round((g.numberOfSubscribers - g.abonentNumber - 1) * g.delayBetweenMessages)));
        g.abonentNumber++;

        // Store delivery
        delivery.save(dtxs.get(g.volume));
      }                                                                                                 

      // Commit distribution DS transaction
      for (DataSourceTransaction dtx : dtxs.values())
        dtx.commit();

      System.out.println("Deliveries generated successfully: distr=" + distributionName + "; tz=" + tz.getID() + "; start=" + startDate + "; end=" + endDate);
    } catch (Throwable e) {
      System.out.println("Deliveries creation failed distrName=" + distributionName + "; TZ=" + tz.getID() + "; start=" + startDate + "; end=" + endDate);
      e.printStackTrace();
      // Rollback distribution DS transaction
      try {
        for (DataSourceTransaction dtx : dtxs.values())
          dtx.rollback();
      } catch (DataSourceException e1) {
        e1.printStackTrace();
      }
    } finally {
      if (stx != null)
        stx.close();

      for (DataSourceTransaction dtx : dtxs.values())
        dtx.close();

      try {
        if (rs != null)
          rs.close();
      } catch (DataSourceException e) {
        e.printStackTrace();
      }
    }
  }

  public void run() {
    System.out.println("Deliveries generation started.");

    for (Iterator<DistributionInfo> distributions = distrInfos.iterator(); distributions.hasNext();) {
      final DistributionInfo info = distributions.next();

      for (Iterator<TimeZone> tzs = timezones.iterator(); tzs.hasNext();) {
        final TimeZone tz = tzs.next();

        // Calculate start and end time in TZ
        final Date now = new Date();
        final Date startDate = getDistrTimeLocal(tz, info.getStartTime());
        final Date endDate = getDistrTimeLocal(tz, info.getEndTime());

        // StartDate will be after now when new day started in TZ
        if (endDate.before(now)) {
          System.out.println("End date in the past: distr=" + info.getDistributionName() + "; tz=" + tz.getID() + "; start=" + startDate + "; end=" + endDate);
          continue;
        }

        // Check deliveries already created
        try {
          if (distributionDS.hasDeliveries(startDate, tz, info.getDistributionName())) {
            System.out.println("Active deliveries found: distr=" + info.getDistributionName() + "; tz=" + tz.getID() + "; start=" + startDate + "; end=" + endDate);
            continue;
          }
        } catch (DataSourceException e) {
          e.printStackTrace();
          return;
        }

        System.out.println("Deliveries generation: distr=" + info.getDistributionName() + "; tz=" + tz.getID() + "; start=" + startDate + "; end=" + endDate);

        try {
          // Create volume groups
          final Map<Integer, VolumeGroup> volumeGroups = createVolumeGroups(info.getDistributionName(), startDate, endDate, tz);
          if (!volumeGroups.isEmpty()) {
            // Schedule
            scheduleDeliveries(info.getDistributionName(), startDate, endDate, volumeGroups, tz);
          }
        } catch (DataSourceException e) {
          e.printStackTrace();
        }
      }
    }

    System.out.println("------------------------------------------------------------------------------------------------------");
  }


  private static Date getDistrTimeLocal(TimeZone tz, Calendar distrTime) {
    // Calculate start time at current day
    final Calendar startDate = Calendar.getInstance(tz);
    // Set to startDate time in tz  (calculate what time is in TZ)
    startDate.setTime(new Date());
    // Move clocks
    startDate.set(Calendar.HOUR_OF_DAY, distrTime.get(Calendar.HOUR_OF_DAY));
    startDate.set(Calendar.MINUTE, distrTime.get(Calendar.MINUTE));
    startDate.set(Calendar.SECOND, distrTime.get(Calendar.SECOND));
    return startDate.getTime();
  }


  private static class VolumeGroup {
    private int volume;
    private double numberOfSubscribers;
    private double totalNumberOfMessages;
    private double delayBetweenMessages;
    private double msgPerSecond;
    //
    private double abonentNumber;
  }


  public static void main(String[] args) {
    DeliveriesDataSource distrDS = null;
    DBSubscriptionDataSource subscrDS = null;
    try {
      final XmlConfig xmlConfig = new XmlConfig();
      xmlConfig.load(new File("conf/config.xml"));

      final Config c = new Config(xmlConfig);

      if (c.getDeliveriesDataSource().equals("db")) {
        PropertiesConfig config = new PropertiesConfig();
        config.load(new File(c.getStorageDistributionSql()));
        distrDS = new DBDistributionDataSource(config);
        ((DBDistributionDataSource)distrDS).init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout());
      } else if (c.getDeliveriesDataSource().equals("file")) {
        distrDS = new FileDeliveriesDataSource(c.getFileStorageStoreDir());
      } else {
        System.out.println("Unknown deliveries storage type: " + c.getDeliveriesDataSource());
        return;
      }

      PropertiesConfig subscrSql = new PropertiesConfig();
      subscrSql.load(new File(c.getStorageSubscriptionSql()));
      subscrDS = new DBSubscriptionDataSource(subscrSql);
      subscrDS.init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout());

      final SmscTimezonesList timezones = new SmscTimezonesList();
      timezones.load(c.getTimezonesFile(), c.getRoutesFile());

      DeliveriesGenerator generator = new DeliveriesGenerator(distrDS, subscrDS);
      for (DistributionInfo distributionInfo : c.getDistrInfos()) {
        System.out.println("Add distr: " + distributionInfo.getDistributionName());
        generator.addDistribution(distributionInfo);
      }

      for (SmscTimezone tz : timezones.getTimezones()) {
        System.out.println("Add TZ: " + tz.getName());
        generator.addTimezone(TimeZone.getTimeZone(tz.getName()));
      }

      generator.run();

    } catch (Exception e) {
      e.printStackTrace();
    } finally {
      if (distrDS != null)
        distrDS.shutdown();
      if (subscrDS != null)
        subscrDS.shutdown();      
    }
  }
}
