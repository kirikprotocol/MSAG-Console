package ru.sibinco.smsx.services.calendar.timezones;

import ru.sibinco.smsx.services.calendar.timezones.parsers.RoutesParser;
import ru.sibinco.smsx.services.calendar.timezones.parsers.TimezonesParser;

import java.util.Collection;


/**
 * User: artem
 * Date: Aug 2, 2006
 */

final class TimezonesXMLParser {

  public static Timezones parse(final String timezonesXML, final String routesXML) {
    // Read timezones from timezonesXML
    final TimezonesParser timezonesParser = new TimezonesParser();
    timezonesParser.parse(ClassLoader.getSystemResourceAsStream(timezonesXML));
    final Collection timezones = timezonesParser.getTimezones();

    // Add masks to timezones from routesXML
    RoutesParser.parse(ClassLoader.getSystemResourceAsStream(routesXML), timezones);

    return new Timezones(timezones, timezonesParser.getDefaultTimezone());
  }

}
