package com.eyeline.sponsored.distribution.advert.admin;

import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.ResultSet;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsDataSource;
import com.eyeline.sponsored.ds.distribution.advert.impl.db.DBDistributionDataSource;
import com.eyeline.sponsored.ds.distribution.advert.impl.file.deliverystats.FileDeliveryStatDataSource;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.Arguments;
import com.eyeline.utils.IOUtils;

import java.io.File;
import java.io.OutputStream;
import java.io.BufferedOutputStream;

/**
 * User: artem
 * Date: 26.03.2008
 */

public class DeliveryStatsView {

  private static void printHelp() {
    System.out.println("Show stats file.");
    System.out.println("Arguments:");
    System.out.println("-d date (dd-mm-yyyy)");
    System.out.println("-s subscriber");
  }

  public static void main(String args[]) {
    final Arguments parser = new Arguments(args);
    if (parser.containsAttr("help")) {
      printHelp();
      return;
    }

    DeliveryStatsDataSource ds = null;
    try {
      final XmlConfig xmlConfig = new XmlConfig();
      xmlConfig.load(new File("conf/config.xml"));

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
      OutputStream os = null;
      try {
        os = new BufferedOutputStream(System.out);
        rs = ds.getDeliveryStats(parser.getDateAttr("-d", "dd-MM-yyyy"));
        String subscriberAddress = parser.getStringAttr("-s", null);
        if (rs != null) {
          while (rs.next()) {
            DeliveryStat stat = rs.get();
            if (subscriberAddress == null || subscriberAddress.equalsIgnoreCase(stat.getSubscriberAddress()))
              IOUtils.writeString(os, stat.getSubscriberAddress() + " | " + stat.getDelivered() + " | " + stat.getAdvertiserId() + '\n');
          }
        }
      } catch (DataSourceException e) {
        e.printStackTrace();
      } finally {
        if (rs != null)
          rs.close();
        if (os != null)
          os.close();
      }

    } catch (Throwable e) {
      e.printStackTrace();
    } finally {
      if (ds != null)
        ds.shutdown();
    }
  }
}
