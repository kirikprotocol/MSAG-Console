package ru.sibinco.calendarsme.engine.calendar;

import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.sibinco.calendarsme.SmeProperties;
import ru.sibinco.calendarsme.network.OutgoingQueue;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class CalendarSmeEngine {

  private final CalendarRequestProcessor requestProcessor;

  public CalendarSmeEngine(final OutgoingQueue outQueue, final Multiplexor multiplexor) {
    final CalendarMessagesList messagesList = new CalendarMessagesList(SmeProperties.General.CALENDAR_MESSAGES_LIST_MAX_SIZE);

    // Start calendar engine
    final CalendarEngine calendarEngine = new CalendarEngine(outQueue, messagesList);
    calendarEngine.startService();
    calendarEngine.waitStarting();

    // Init request processor
    this.requestProcessor = new CalendarRequestProcessor(messagesList, multiplexor);
  }

  public boolean processMessage(final Message message) {
    return requestProcessor.processRequest(message);
  }
}
