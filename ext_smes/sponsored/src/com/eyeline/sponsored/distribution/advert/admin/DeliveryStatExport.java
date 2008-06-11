package com.eyeline.sponsored.distribution.advert.admin;

import com.eyeline.sponsored.distribution.advert.config.Config;
import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.ResultSet;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsDataSource;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsQuery;
import com.eyeline.sponsored.ds.distribution.advert.impl.db.DBDistributionDataSource;
import com.eyeline.sponsored.ds.distribution.advert.impl.file.deliverystats.FileDeliveryStatDataSource;
import com.eyeline.sponsored.utils.CalendarUtils;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.Arguments;
import com.eyeline.utils.IOUtils;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

/**
 * User: artem
 * Date: 05.02.2008
 */

public class DeliveryStatExport {

  public static void exportStat(DeliveryStatsDataSource ds, Date fromDate, Date toDate, float coeff, final Integer advId, final Integer notAdvId, OutputStream os)  {
    ResultSet<DeliveryStat> rs = null;
    try {

      rs = ds.aggregateDeliveryStats(fromDate, toDate, new DeliveryStatsQuery() {
        public boolean isAllowed(DeliveryStat stat) {
          if (advId != null && stat.getAdvertiserId() != advId)
            return false;

          if (notAdvId != null && stat.getAdvertiserId() == notAdvId)
            return false;

          return true;
        }
      });

      if (rs == null)
        return;

      while(rs.next()) {
        final DeliveryStat stat = rs.get();

        if (stat.getDelivered() > 0) {
          float cost = stat.getDelivered() * 100000 * coeff / 100000;
          final StringBuilder st = new StringBuilder(40);
          st.append(stat.getSubscriberAddress()).append('|').append(cost).append('\n');
          IOUtils.writeString(os, st.toString(), st.length());
        }
      }

    } catch (DataSourceException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      try {
      if (rs != null)
        rs.close();
      } catch (DataSourceException e) {
        e.printStackTrace();
      }

    }
  }

  private static void printHelp() {
    System.out.println("Export statistics to file");
    System.out.println("Arguments:");
    System.out.println("-f from date (dd-mm-yyyy)");
    System.out.println("-t to date (dd-mm-yyyy)");
    System.out.println("-c cost");
    System.out.println("-i days interval");
    System.out.println("-d output directory");
    System.out.println("-a advertiser id");
    System.out.println("-n advertiser id not equals to");
  }

  /**
   * Arguments:
   * -f from date (dd-mm-yyyy)
   * -t to date (dd-mm-yyyy)
   * -c cost
   * -i days interval
   * -d output directory
   * -a advertiser id
   * -n advertiser id not equals to
   *
   * Example 1:
   * Export stats from 14-10-2007 to 20-10-2007 with cost = 0.01
   * -f 14-10-2007 -t 20-10-2007 -c 0.01
   *
   * Example 2:
   * Export stats by all distributions from 13-10-2007 to 20-10-2007 with cost = 0.01
   * -t 20-10-2007 -i 7 -c 0.01
   *
   * Example 3:
   * Export stats from last 7 days with cost = 10.28
   * -i 7 -c 10.28
   *
   * Example 4:
   * Export all stats with cost 0.05
   * -c 0.05
   * @param args
   */

  public static void main(String[] args) {
    final Request req = new Request();

    DeliveryStatsDataSource ds = null;
    OutputStream os = null;
    try {

      final Arguments parser = new Arguments(args);

      req.setFromDate(parser.getStringAttr("-f", null));
      req.setToDate(parser.getStringAttr("-t", null));
      req.setCost(parser.getStringAttr("-c", null));
      req.setInterval(parser.getStringAttr("-i", null));
      req.setDir(parser.getStringAttr("-d", null));
      if (parser.containsAttr("-a"))
        req.setAdvertiserId(parser.getIntAttr("-a"));
      if (parser.containsAttr("-n"))
        req.setNotAdvertiserId(parser.getIntAttr("-n"));
      if (parser.containsAttr("help")) {
        printHelp();
        return;
      }

      req.validate();

      req.prepare();

      System.out.println("Delivery stat export");
      System.out.println("From date " + req.getFromDate());
      System.out.println("To date " + req.getToDate());
      System.out.println("Cost " + req.getCost());
      if (req.getAdvertiserId() != null)
        System.out.println("Advertiser " + req.getAdvertiserId());
      if (req.getNotAdvertiserId() != null)
        System.out.println("Advertiser does not equals " + req.getNotAdvertiserId());

      final XmlConfig xmlConfig = new XmlConfig();
      xmlConfig.load(new File("conf/config.xml"));

      final Config c = new Config(xmlConfig);

      if (c.getDeliveryStatsDataSource().equals("db")) {
        ds = new DBDistributionDataSource(new PropertiesConfig(c.getStorageDistributionSql()));
        ((DBDistributionDataSource)ds).init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout());
      } else if (c.getDeliveryStatsDataSource().equals("file")) {
        ds = new FileDeliveryStatDataSource(c.getFileStorageStoreDir());
      } else {
        System.out.println("Unknown delivery stats storage type: " + c.getDeliveryStatsDataSource());
        return;
      }

      final SimpleDateFormat df = new SimpleDateFormat("dd_MM_yyyy");
      os = new BufferedOutputStream(new FileOutputStream(new File(req.getDir(), "export." + df.format(req.getFromDate()) + '-' + df.format(CalendarUtils.getPrevDayStart(req.getToDate())) + ".csv")));

      long time = System.currentTimeMillis();

      exportStat(ds, req.getFromDate(), req.getToDate(), req.getCost(), req.getAdvertiserId(), req.getNotAdvertiserId(), os);

      os.flush();

      System.out.println("Stats aggregated in " + (System.currentTimeMillis() - time) + " ms");

    } catch (Throwable e) {
      e.printStackTrace();
    } finally {
      try {
        if (os != null)
          os.close();
      } catch (Throwable e) {

      }
      if (ds != null)
        ds.shutdown();
    }
  }

  private static class Request {
    private static final SimpleDateFormat df = new SimpleDateFormat("dd-MM-yyyy");
    Date fromDate;
    Date toDate;
    float cost;
    int interval;
    String dir;
    Integer advertiserId;
    Integer notAdvertiserId;

    private Request() {
      this.fromDate = null;
      this.toDate = CalendarUtils.getDayStart(new Date());
      this.cost = -1;
      this.interval = -1;
      this.advertiserId = null;
      this.notAdvertiserId = null;
      this.dir = ".";
    }

    public Date getFromDate() {
      return fromDate;
    }

    public void setFromDate(String fromDate) {
      try {
        this.fromDate = CalendarUtils.getDayStart(df.parse(fromDate));
      } catch (ParseException e) {
        throw new IllegalArgumentException("Invalid start date. Format is dd-mm-yyyy");
      }
    }

    public Date getToDate() {
      return toDate;
    }

    public void setToDate(String toDate) {
      try {
        this.toDate = CalendarUtils.getNextDayStart(df.parse(toDate));
      } catch (ParseException e) {
        throw new IllegalArgumentException("Invalid end date. Format is dd-mm-yyyy");
      }
    }

    public float getCost() {
      return cost;
    }

    public void setCost(String cost) {
      try {
        this.cost = Float.parseFloat(cost);
      } catch (NumberFormatException e) {
        throw new IllegalArgumentException("Invalid cost");
      }
    }

    public int getInterval() {
      return interval;
    }

    public void setInterval(String interval) {
      try {
      this.interval = Integer.parseInt(interval);
      } catch (NumberFormatException e) {
        throw new IllegalArgumentException("Invalid interval");
      }
    }

    public Integer getAdvertiserId() {
      return advertiserId;
    }

    public void setAdvertiserId(Integer advertiserId) {
      this.advertiserId = advertiserId;
    }

    public Integer getNotAdvertiserId() {
      return notAdvertiserId;
    }

    public void setNotAdvertiserId(Integer notAdvertiserId) {
      this.notAdvertiserId = notAdvertiserId;
    }

    public String getDir() {
      return dir;
    }

    public void setDir(String dir) {
      this.dir = dir;
    }

    public void validate() {
      if (fromDate != null && toDate != null && toDate.before(fromDate))
        throw new IllegalArgumentException("From date before to date");

      if (fromDate != null && interval >= 0)
        throw new IllegalArgumentException("Can't use from date and interval arguments at the same time");

      if (interval == 0)
        throw new IllegalArgumentException("Interval is zero");

      if (cost == -1)
        throw new IllegalArgumentException("Cost is empty");
    }

    public void prepare() {
      if (fromDate == null) {
        if (interval > 0) {
          final Calendar c = Calendar.getInstance();
          c.setTime(toDate);
          c.set(Calendar.DATE, c.get(Calendar.DATE) - interval);
          fromDate = c.getTime();
        } else {
          fromDate = new Date(0);
        }
      }
    }
  }

}
