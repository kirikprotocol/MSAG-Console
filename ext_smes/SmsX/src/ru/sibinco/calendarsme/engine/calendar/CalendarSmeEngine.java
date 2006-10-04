package ru.sibinco.calendarsme.engine.calendar;

import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.sibinco.calendarsme.network.OutgoingQueue;
import ru.sibinco.calendarsme.utils.Utils;

import java.util.Properties;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class CalendarSmeEngine {

  private final CalendarRequestProcessor requestProcessor;

  public CalendarSmeEngine(final Properties config, final OutgoingQueue outQueue, final Multiplexor multiplexor) {
    final CalendarMessagesList messagesList = new CalendarMessagesList(Utils.loadInt(config, "calendar.messages.list.max.size"));

    // Start calendar engine
    final CalendarEngine calendarEngine = new CalendarEngine(config, outQueue, messagesList);
    calendarEngine.startService();
    calendarEngine.waitStarting();

    // Init request processor
    this.requestProcessor = new CalendarRequestProcessor(config, messagesList, multiplexor);
  }

  public boolean processMessage(final Message message) {
    return requestProcessor.processRequest(message);
  }
}
