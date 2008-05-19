package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.smpp.OutgoingQueue;

/**
 * User: artem
 * Date: 05.07.2007
 */

public class CalendarServiceFactory {

  private static CalendarServiceImpl impl;

  public static synchronized CalendarService getCalendarService(String configDir, OutgoingQueue outQueue) {
    if (impl == null)
      impl = new CalendarServiceImpl(configDir, outQueue);
    return impl;
  }
}
