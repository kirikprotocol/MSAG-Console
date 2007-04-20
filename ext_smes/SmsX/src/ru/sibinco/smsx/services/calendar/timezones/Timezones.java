package ru.sibinco.smsx.services.calendar.timezones;

import org.apache.log4j.Category;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.services.calendar.CalendarService;
import ru.sibinco.smsx.utils.Service;

import java.io.File;
import java.net.URI;
import java.util.Collection;
import java.util.Iterator;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class Timezones {

  private static final Category log = Category.getInstance(Timezones.class);

  private static Timezones instance;
  private static final Object instanceLock = new Object();

  public static String getTimezoneByAbonent(final String abonentAddr) {
    synchronized(instanceLock) {
      return instance.getTimezoneByAbonentInternal(abonentAddr);
    }
  }

  public static void init() {
    try {
      instance = TimezonesXMLParser.parse(CalendarService.Properties.Timezones.TIMEZONES_XML, CalendarService.Properties.Timezones.ROUTES_XML);
    } catch (Exception e) {
      throw new InitializationException("Can't load timezonez or routes. " + e.getMessage());
    }
    new TimezonesReloader().startService();
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
        wait((!errorOccured) ? CalendarService.Properties.Timezones.RELOAD_TIMEZONES_INTERVAL : 15000);
      } catch (InterruptedException e) {
        log.error("Interrupted", e);
      }

      final File tf = new File(URI.create(ClassLoader.getSystemResource(CalendarService.Properties.Timezones.TIMEZONES_XML).toString()));
      final File rf = new File(URI.create(ClassLoader.getSystemResource(CalendarService.Properties.Timezones.ROUTES_XML).toString()));

      if (timezonesLastModifiedTime != tf.lastModified() || routesLastModifiedTime != rf.lastModified()) {
        try {
          final Timezones timezones = TimezonesXMLParser.parse(CalendarService.Properties.Timezones.TIMEZONES_XML, CalendarService.Properties.Timezones.ROUTES_XML);

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

    CalendarService.Properties.Timezones.TIMEZONES_XML = "timezones.xml";
    CalendarService.Properties.Timezones.ROUTES_XML = "routes.xml";
    Timezones.init();
    System.out.println(Timezones.getTimezoneByAbonent(".1.1.79133105665"));
    System.out.println(Timezones.getTimezoneByAbonent("+79137777000"));
    System.out.println(Timezones.getTimezoneByAbonent(".1.1.9001"));
  }
}
