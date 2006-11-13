package ru.sibinco.otasme.engine.smscenters;


import org.apache.log4j.Category;
import ru.sibinco.otasme.SmeProperties;
import ru.sibinco.otasme.utils.Service;

import java.io.File;
import java.net.URI;
import java.util.Collection;
import java.util.Iterator;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class SMSCenters {

  private static final Category log = Category.getInstance(SMSCenters.class);

  private static SMSCenters instance;
  private static final Object instanceLock = new Object();

  static {
    instance = SMSCentersXMLParser.parse(SmeProperties.Templates.SMSCENTERS_XML, SmeProperties.Templates.ROUTES_XML);
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
      final SMSCenter smsc =  (SMSCenter)iterator.next();
      if (smsc.allowNumber(abonentAddr))
        return smsc.getNumber();
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
        wait((!errorOccured) ? SmeProperties.Templates.RELOAD_INTERVAL : 15000);
      } catch (InterruptedException e) {
        log.error("Interrupted", e);
      }

      final File tf = new File(URI.create(ClassLoader.getSystemResource(SmeProperties.Templates.SMSCENTERS_XML).toString()));
      final File rf = new File(URI.create(ClassLoader.getSystemResource(SmeProperties.Templates.ROUTES_XML).toString()));

      if (timezonesLastModifiedTime != tf.lastModified() || routesLastModifiedTime != rf.lastModified()) {
        try {
          final SMSCenters centers = SMSCentersXMLParser.parse(SmeProperties.Templates.SMSCENTERS_XML, SmeProperties.Templates.ROUTES_XML);

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
    System.out.println(SMSCenters.getSMSCenterNumberByAbonent("1.2.3.4"));
    System.out.println(SMSCenters.getSMSCenterNumberByAbonent(".0.1.3"));
    System.out.println(SMSCenters.getSMSCenterNumberByAbonent(".1.1.9001"));
  }
}
