package ru.sibinco.smsx.engine.service.calendar;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;

/**
 * User: artem
 * Date: 05.07.2007
 */

public class CalendarServiceFactory {

  private static CalendarServiceImpl impl;

  public static synchronized CalendarService getCalendarService(XmlConfig config, OutgoingQueue outQueue) {
    if (impl == null)
      impl = new CalendarServiceImpl(config, outQueue);
    return impl;
  }
}
