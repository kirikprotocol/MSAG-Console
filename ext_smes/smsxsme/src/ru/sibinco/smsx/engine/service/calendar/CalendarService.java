package ru.sibinco.smsx.engine.service.calendar;

import ru.sibinco.smsx.engine.service.ServiceController;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarCheckMessageStatusCmd;

/**
 * User: artem
 * Date: 29.06.2007
 */

public interface CalendarService extends ServiceController, CalendarSendMessageCmd.Receiver, CalendarCheckMessageStatusCmd.Receiver {
}
