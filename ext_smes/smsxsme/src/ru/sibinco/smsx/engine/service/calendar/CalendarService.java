package ru.sibinco.smsx.engine.service.calendar;

import ru.sibinco.smsx.engine.service.Service;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarCheckMessageStatusCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarHandleReceiptCmd;

/**
 * User: artem
 * Date: 29.06.2007
 */

public interface CalendarService extends Service, CalendarSendMessageCmd.Receiver,
                                                            CalendarCheckMessageStatusCmd.Receiver,
                                                            CalendarHandleReceiptCmd.Receiver {
}
