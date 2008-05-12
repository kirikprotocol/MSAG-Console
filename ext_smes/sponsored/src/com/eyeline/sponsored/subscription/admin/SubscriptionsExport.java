package com.eyeline.sponsored.subscription.admin;

import com.eyeline.sponsored.ds.DataSourceException;
import com.eyeline.sponsored.ds.DataSourceTransaction;
import com.eyeline.sponsored.ds.ResultSet;
import com.eyeline.sponsored.ds.subscription.Subscription;
import com.eyeline.sponsored.ds.subscription.impl.db.DBSubscriptionDataSource;
import com.eyeline.sponsored.subscription.config.Config;
import com.eyeline.utils.config.ConfigException;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.IOUtils;
import ru.sibinco.smsc.utils.subjects.SmscSubject;
import ru.sibinco.smsc.utils.subjects.SmscSubjectsList;
import ru.sibinco.smsc.utils.subjects.SmscSubjectsListException;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

/**
 * User: artem
 * Date: 28.04.2008
 */

public class SubscriptionsExport {

  private static void printHelp() {
    System.out.println("Export subscription info.");
    System.out.println("Arguments:");
    System.out.println("-a date (dd-mm-yyyy)");
    System.out.println("-d distribution name");
    System.out.println("-f routes file");
  }

  public static void main(String[] args) {

    if (args.length % 2 != 0) {
      printHelp();
      return;
    }

    Request req = new Request();
    for (int i=0; i + 1 < args.length; i+=2) {
      try {
        if (args[i].equalsIgnoreCase("-a"))
          req.setDate(args[i + 1]);
        else if (args[i].equalsIgnoreCase("-d"))
          req.setDistributionName(args[i + 1]);
        else if (args[i].equalsIgnoreCase("-f"))
          req.setRoutesFile(args[i + 1]);
        else {
          printHelp();
          return;
        }
      } catch (IllegalArgumentException e) {
        System.out.println(e.getMessage());
        printHelp();
        return;
      }
    }

    DBSubscriptionDataSource ds = null;
    OutputStream os = null;
    BufferedReader r = null;
    try {
      final XmlConfig xmlConfig = new XmlConfig();
      xmlConfig.load(new File("conf/config.xml"));

      Config c = new Config(xmlConfig);

      if (req.getRoutesFile() == null)
        req.setRoutesFile(c.getRoutesFile());

      if (req.getDate() == null)
        req.setDate(new Date());

      // Init data source
      ds = new DBSubscriptionDataSource(new PropertiesConfig(c.getSubscriptionSql()));
      ds.init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout(), c.getStoragePoolSize());

      final SmscSubjectsList subjects = new SmscSubjectsList();
      subjects.load(req.getRoutesFile());
      final SmscSubject unknownSubject = new SmscSubject("Unknown");

      final Map<SmscSubject, Counter> stats = new HashMap<SmscSubject, Counter>(subjects.getSubjects().size());

      DataSourceTransaction tx = null;

      try {
        tx = ds.createTransaction();
        ResultSet<Subscription> subscriptions = ds.lookupActiveSubscriptions(req.getDistributionName(), req.getDate(), tx);

        while (subscriptions.next()) {
          String address = subscriptions.get().getSubscriberAddress();
          SmscSubject s = subjects.getSubjectByAddress(address);
          if (s == null) {
            s = unknownSubject;
            System.err.println(address);
          }
          Counter counter = stats.get(s);
          if (counter == null) {
            counter = new Counter();
            stats.put(s, counter);
          }
          counter.inc();
        }

      } catch (DataSourceException e) {
        e.printStackTrace();
      } finally {
        if (tx != null)
          tx.close();
      }

      os = new BufferedOutputStream(System.out);

      for (Map.Entry<SmscSubject, Counter> e : stats.entrySet()) {
        IOUtils.writeString(os, e.getKey().getId() + '|' + e.getValue().getValue() + '\n');
      }

    } catch (SmscSubjectsListException e) {
      e.printStackTrace();
    } catch (DataSourceException e) {
      e.printStackTrace();
    } catch (ConfigException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (ds != null)
        ds.shutdown();
      if (os != null)
        try {
          os.close();
        } catch (IOException e) {
          e.printStackTrace();
        }
    }
  }

  private static class Request {
    private static final SimpleDateFormat df = new SimpleDateFormat("dd-MM-yyyy");

    private String distributionName;
    private Date date;
    private String routesFile;

    public String getDistributionName() {
      return distributionName;
    }

    public void setDistributionName(String distributionName) {
      this.distributionName = distributionName;
    }

    public Date getDate() {
      return date;
    }

    public void setDate(Date date) {
      this.date = date;
    }

    public void setDate(String date) {
      try {
        this.date = df.parse(date);
      } catch (ParseException e) {
        throw new IllegalArgumentException(e);
      }
    }

    public String getRoutesFile() {
      return routesFile;
    }

    public void setRoutesFile(String routesFile) {
      this.routesFile = routesFile;
    }
  }

  private static class Counter {
    int value = 0;

    public void inc() {
      value++;
    }

    public int getValue() {
      return value;
    }
  }
}
