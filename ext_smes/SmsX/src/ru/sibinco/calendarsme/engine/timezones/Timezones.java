package ru.sibinco.calendarsme.engine.timezones;

import ru.sibinco.calendarsme.utils.Service;
import ru.sibinco.calendarsme.utils.Utils;

import java.util.Collection;
import java.util.Iterator;
import java.util.Properties;
import java.util.Date;
import java.io.File;
import java.net.URI;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class Timezones {

  private static final Category log = Category.getInstance(Timezones.class);

  private static final int RELOAD_INTERVAL;
  private static final String TEMPLATES_XML;
  private static final String ROUTES_XML;

  private static Timezones instance;
  private static final Object instanceLock = new Object();

  static {
    final Properties config = Utils.loadConfig("sme.properties");
    RELOAD_INTERVAL = Utils.loadInt(config, "request.processor.reload.timezones.interval");
    TEMPLATES_XML = Utils.loadString(config, "request.processor.timezones.xml");
    ROUTES_XML = Utils.loadString(config, "request.processor.routes.xml");

    instance = TimezonesXMLParser.parse(TEMPLATES_XML, ROUTES_XML);
    new TimezonesReloader().startService();
  }

  public static String getTimezoneByAbonent(final String abonentAddr) {
    synchronized(instanceLock) {
      return instance.getTimezoneByAbonentInternal(abonentAddr);
    }
  }

  private final Collection timezones;
  private final String defaultTimezone;

  Timezones(Collection timezones, final String defaultTimezone) {
    this.timezones = timezones;
    this.defaultTimezone = defaultTimezone;
  }

  private String getTimezoneByAbonentInternal(final String abonentAddr) {
    for (Iterator iterator = timezones.iterator(); iterator.hasNext();) {
      final Timezone tz =  (Timezone)iterator.next();
      if (tz.allowNumber(abonentAddr))
        return tz.getName();
    }
    return defaultTimezone;
  }

  private static class TimezonesReloader extends Service {
    private boolean errorOccured = false;
    private long timezonesLastModifiedTime = -1;
    private long routesLastModifiedTime = -1;

    TimezonesReloader() {
      super(log);
    }

    public synchronized void iterativeWork() {
      try {
        wait((!errorOccured) ? RELOAD_INTERVAL : 15000);
      } catch (InterruptedException e) {
        log.error("Interrupted", e);
      }

      final File tf = new File(URI.create(ClassLoader.getSystemResource(TEMPLATES_XML).toString()));
      final File rf = new File(URI.create(ClassLoader.getSystemResource(ROUTES_XML).toString()));

      if (timezonesLastModifiedTime != tf.lastModified() || routesLastModifiedTime != rf.lastModified()) {
        try {
          final Timezones timezones = TimezonesXMLParser.parse(TEMPLATES_XML, ROUTES_XML);

          if (timezones != null) {
            log.info("Timezones have been reloaded");
            errorOccured = false;
            timezonesLastModifiedTime = tf.lastModified();
            routesLastModifiedTime = rf.lastModified();

            synchronized(instanceLock) {
              instance = timezones;
            }
          } else
            log.info("Can't read timezones. Error.");

        } catch (Throwable e) {
          log.error("Error reading timezones!!!", e);
          errorOccured = true;
        }
      }
    }
  }

  public static void main(String args[]) {
    System.out.println(Timezones.getTimezoneByAbonent(".1.1.79133105665"));
    System.out.println(Timezones.getTimezoneByAbonent(".1.1.79135019090"));
    System.out.println(Timezones.getTimezoneByAbonent(".1.1.9001"));
  }
}
