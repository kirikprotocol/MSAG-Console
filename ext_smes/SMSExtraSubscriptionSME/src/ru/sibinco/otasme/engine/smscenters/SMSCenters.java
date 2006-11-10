package ru.sibinco.otasme.engine.smscenters;


import java.util.Collection;
import java.util.Iterator;
import java.util.Properties;
import java.io.File;
import java.net.URI;

import org.apache.log4j.Category;
import ru.sibinco.otasme.utils.Utils;
import ru.sibinco.otasme.utils.Service;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class SMSCenters {

  private static final Category log = Category.getInstance(SMSCenters.class);

  private static final int RELOAD_INTERVAL;
  private static final String SMSCENTERS_XML;
  private static final String ROUTES_XML;

  private static SMSCenters instance;
  private static final Object instanceLock = new Object();

  static {
    final Properties config = Utils.loadConfig("sme.properties");
    RELOAD_INTERVAL = Utils.loadInt(config, "sme.engine.reload.smscenters.interval");
    SMSCENTERS_XML = Utils.loadString(config, "sme.engine.smscenters.xml");
    ROUTES_XML = Utils.loadString(config, "sme.engine.routes.xml");

    instance = SMSCentersXMLParser.parse(SMSCENTERS_XML, ROUTES_XML);
    new SMSCentersReloader().startService();
  }

  public static String getSMSCenterNumberByAbonent(final String abonentAddr) {
    synchronized(instanceLock) {
      return instance.getSMSCenterNumberByAbonentInternal(abonentAddr);
    }
  }

  private final Collection timezones;
  private final String defaultTimezone;

  SMSCenters(Collection timezones, final String defaultTimezone) {
    this.timezones = timezones;
    this.defaultTimezone = defaultTimezone;
  }

  private String getSMSCenterNumberByAbonentInternal(final String abonentAddr) {
    for (Iterator iterator = timezones.iterator(); iterator.hasNext();) {
      final SMSCenter tz =  (SMSCenter)iterator.next();
      if (tz.allowNumber(abonentAddr))
        return tz.getNumber();
    }
    return defaultTimezone;
  }

  private static class SMSCentersReloader extends Service {
    private boolean errorOccured = false;
    private long timezonesLastModifiedTime = -1;
    private long routesLastModifiedTime = -1;

    SMSCentersReloader() {
      super(log);
    }

    public synchronized void iterativeWork() {
      try {
        wait((!errorOccured) ? RELOAD_INTERVAL : 15000);
      } catch (InterruptedException e) {
        log.error("Interrupted", e);
      }

      final File tf = new File(URI.create(ClassLoader.getSystemResource(SMSCENTERS_XML).toString()));
      final File rf = new File(URI.create(ClassLoader.getSystemResource(ROUTES_XML).toString()));

      if (timezonesLastModifiedTime != tf.lastModified() || routesLastModifiedTime != rf.lastModified()) {
        try {
          final SMSCenters centers = SMSCentersXMLParser.parse(SMSCENTERS_XML, ROUTES_XML);

          if (centers != null) {
            log.info("SMSCenters have been reloaded");
            errorOccured = false;
            timezonesLastModifiedTime = tf.lastModified();
            routesLastModifiedTime = rf.lastModified();

            synchronized(instanceLock) {
              instance =centers;
            }
          } else
            log.info("Can't read SMSCenters. Error.");

        } catch (Throwable e) {
          log.error("Error reading timezones!!!", e);
          errorOccured = true;
        }
      }
    }
  }

  public static void main(String args[]) {
    System.out.println(SMSCenters.getSMSCenterNumberByAbonent(".1.1.79133105665"));
    System.out.println(SMSCenters.getSMSCenterNumberByAbonent(".1.1.79135019090"));
    System.out.println(SMSCenters.getSMSCenterNumberByAbonent(".1.1.9001"));
  }
}
