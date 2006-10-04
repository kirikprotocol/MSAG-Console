package ru.sibinco.calendarsme.engine.timezones;

import ru.sibinco.calendarsme.engine.timezones.parsers.RoutesParser;
import ru.sibinco.calendarsme.engine.timezones.parsers.TimezonesParser;

import java.util.Iterator;
import java.util.SortedMap;
import java.util.TreeMap;


/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class TimezonesXMLParser {
  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(TimezonesXMLParser.class);

  public static Timezones parse(final String timezonesXML, final String routesXML) {
    final SortedMap timezones = TimezonesParser.parse(ClassLoader.getSystemResourceAsStream(timezonesXML));
    final SortedMap routes = RoutesParser.parse(ClassLoader.getSystemResourceAsStream(routesXML));

    final SortedMap abonentsTimezones = new TreeMap();

    for(Iterator iter = routes.keySet().iterator(); iter.hasNext();) {
      final String phoneMask = (String)iter.next();
      final String timezoneName = (String)routes.get(phoneMask);
      final String timezone = (String)timezones.get(timezoneName);

      if (timezone == null) {
        Log.warn("!!! WARNING: " + routesXML + " has abonents with unknown timezone: " + timezoneName);
        continue;
      }

      abonentsTimezones.put(preparePhoneMask(phoneMask), timezone);
    }

    return new Timezones(abonentsTimezones, (String)timezones.get("defaultTimezone"));
  }

  private static String preparePhoneMask(final String phoneMask) {
    String result = phoneMask.replace('?', '.');

    if (result.charAt(0)=='+')
      result = "\\+" + result.substring(1);

    return result;
  }
}
