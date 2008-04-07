package com.eyeline.sponsored.distribution.advert.admin;

import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.sponsored.ds.distribution.advert.impl.db.DBDistributionDataSource;
import com.eyeline.sponsored.ds.distribution.advert.impl.file.deliverystats.FileDeliveryStatDataSource;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsDataSource;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;
import com.eyeline.sponsored.ds.ResultSet;
import com.eyeline.sponsored.ds.DataSourceException;

import java.util.Date;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.io.File;

/**
 * User: artem
 * Date: 26.03.2008
 */

public class DeliveryStatsView {
  public static void main(String args[]) {
    // Flags
    // -d date in format <yyyy-MM-dd>
    // -s subscriber
    final Args a = new Args(args);

    DeliveryStatsDataSource ds = null;
    try {
      final XmlConfig xmlConfig = new XmlConfig(new File("conf/config.xml"));
      xmlConfig.load();

      final Config c = new Config(xmlConfig);

      if (c.getDeliveryStatsDataSource().equals("db")) {
        ds = new DBDistributionDataSource(new PropertiesConfig(c.getStorageDistributionSql()));
        ((DBDistributionDataSource)ds).init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout(), c.getStoragePoolSize());
      } else if (c.getDeliveryStatsDataSource().equals("file")) {
        ds = new FileDeliveryStatDataSource(c.getFileStorageStoreDir());
      } else {
        System.out.println("Unknown delivery stats storage type: " + c.getDeliveryStatsDataSource());
        return;
      }

      ResultSet<DeliveryStat> rs = null;
      try {
        rs = ds.getDeliveryStats(a.date);
        if (rs != null) {
          while (rs.next()) {
            DeliveryStat stat = rs.get();
            if (a.subscriber == null || a.subscriber.equalsIgnoreCase(stat.getSubscriberAddress()))
              System.out.println(stat.getSubscriberAddress() + " | " + stat.getDelivered());
          }
        }
      } catch (DataSourceException e) {
        e.printStackTrace();
      } finally {
        if (rs != null)
          rs.close();
      }

    } catch (Throwable e) {
      e.printStackTrace();
    } finally {
      if (ds != null)
        ds.shutdown();
    }
  }

  private static class Args {
    private static final SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd");
    private String subscriber;
    private Date date;

    public Args(String args[]) {
      subscriber = null;
      date = null;

      if (args.length >= 2) {
        if (args[0].equals("-d")) {
          try {
            date = df.parse(args[1]);
          } catch (ParseException e) {
            throw new IllegalArgumentException("Invalid date format. Should be yyyy-MM-dd");
          }
        }
        if (args[0].equals("-s")) {
          subscriber = args[1];
        }
      }

      if (args.length >= 4) {
        if (args[2].equals("-d")) {
          try {
            date = df.parse(args[3]);
          } catch (ParseException e) {
            throw new IllegalArgumentException("Invalid date format. Should be yyyy-MM-dd");
          }
        }
        if (args[2].equals("-s")) {
          subscriber = args[3];
        }
      }

      if (date == null)
        throw new IllegalArgumentException("Date is empty");
    }
  }
}
