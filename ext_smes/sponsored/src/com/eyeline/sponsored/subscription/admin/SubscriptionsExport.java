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

  public static void main(String[] args) {

    Request req = new Request();
    for (int i=0; i + 1 < args.length; i+=2) {
      if (args[i].equalsIgnoreCase("-a")) {
        req.setDate(args[i + 1]);
      }
      if (args[i].equalsIgnoreCase("-d")) {
        req.setDistributionName(args[i + 1]);
      }
      if (args[i].equalsIgnoreCase("-f")) {
        req.setRoutesFile(args[i + 1]);
      }
    }

    DBSubscriptionDataSource ds = null;
    BufferedReader r = null;
    try {
      final XmlConfig xmlConfig = new XmlConfig(new File("conf/config.xml"));
      xmlConfig.load();

      Config c = new Config(xmlConfig);

      if (req.getRoutesFile() == null)
        req.setRoutesFile(c.getRoutesFile());

      if (req.getDate() == null)
        req.setDate(new Date());

      // Init data source
      ds = new DBSubscriptionDataSource(new PropertiesConfig(c.getSubscriptionSql()));
      ds.init(c.getStorageDriver(), c.getStorageUrl(), c.getStorageLogin(), c.getStoragePwd(), c.getStorageConnTimeout(), c.getStoragePoolSize());

      final SmscSubjectsList subjects = new SmscSubjectsList(req.getRoutesFile());
      final SmscSubject unknownSubject = new SmscSubject("Unknown");
//      final SmscTimezonesList tz = new SmscTimezonesList(c.getTimezonesFile(), c.getRoutesFile());

//      SubscriptionProcessor.init(ds, tz);
//
//
//      r = new BufferedReader(new FileReader("spons.txt"));
//
//      String s = r.readLine();
//      while(s != null) {
//        SubscriptionProcessor.getInstance().subscribe(s, "sponsored2", 10);
//        s = r.readLine();
//      }
////
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

      for (Map.Entry<SmscSubject, Counter> e : stats.entrySet()) {
        System.out.println(e.getKey().getId() + '|' + e.getValue().getValue());
      }

    } catch (SmscSubjectsListException e) {
      e.printStackTrace();
    } catch (DataSourceException e) {
      e.printStackTrace();
    } catch (ConfigException e) {
      e.printStackTrace();
//    } catch (FileNotFoundException e) {
//      e.printStackTrace();
//    } catch (IOException e) {
//      e.printStackTrace();
//    } catch (SmscTimezonesListException e) {
//      e.printStackTrace();
//    } catch (ProcessorException e) {
//      e.printStackTrace();
    } finally {
      if (ds != null)
        ds.shutdown();
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
