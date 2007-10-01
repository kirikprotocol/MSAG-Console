package ru.sibinco.smsx.engine.service.calendar;

import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;

/**
 * User: artem
 * Date: 05.07.2007
 */

public class CalendarServiceFactory {

  private static CalendarServiceImpl impl;

  public static synchronized CalendarService getCalendarService(String configDir, SMPPOutgoingQueue outQueue) {
    if (impl == null)
      impl = new CalendarServiceImpl(configDir, outQueue);
    return impl;
  }
}
