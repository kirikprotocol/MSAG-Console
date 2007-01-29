/**
 * User: artem
 * Date: Jul 28, 2006
 */

package ru.sibinco.smsx.services.calendar;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.sibinco.smsx.network.OutgoingQueue;
import ru.sibinco.smsx.services.ServiceProcessor;
import ru.sibinco.smsx.services.calendar.timezones.Timezones;
import ru.sibinco.smsx.utils.BlockingQueue;

import java.sql.SQLException;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

final class CalendarRequestProcessor extends ServiceProcessor {
  private static final org.apache.log4j.Category log = org.apache.log4j.Category.getInstance(CalendarRequestProcessor.class);

  private final CalendarMessagesList messagesList;
  private final BlockingQueue inQueue;

  public CalendarRequestProcessor(final BlockingQueue inQueue, final OutgoingQueue outQueue, final CalendarMessagesList messagesList, Multiplexor multiplexor) {
    super(log, outQueue, multiplexor);
    this.messagesList = messagesList;
    this.inQueue = inQueue;
  }

  public void iterativeWork() {
    try {
      processMessage((ParsedMessage)inQueue.getObject());
    } catch (Throwable e) {
      log.error("Error", e);
    }
  }

  private void processMessage(final ParsedMessage parsedMessage)  {
    final Message message = parsedMessage.getMessage();

    try {
      log.info("=====================================================================================");
      log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

      final CalendarRequestParser.ParseResult parseResult = parsedMessage.getParseResult();

      final Date sendDate = (parseResult.getType() == CalendarRequestParser.ATF_REQUEST) ? parseResult.getDate() :
        changeDateAccordingTimezone(message.getSourceAddress(), parseResult.getDate());

      // Check send date
      if (sendDate.before(new Date())) {
        sendResponse(message, Data.ESME_RX_P_APPN);
        sendMessage(message.getDestinationAddress(), message.getSourceAddress(), CalendarService.Properties.CALENDAR_SEND_DATE_IS_IN_THE_PAST);
        return;
      }

      processMessage(message, new CalendarMessage(message.getSourceAddress(), message.getDestinationAddress(), sendDate, parseResult.getMessage()));

    } catch (Exception e) {
      log.error(e);
      sendResponse(message, Data.ESME_RSYSERR);
    }
  }

  private Date changeDateAccordingTimezone(final String abonent, final Date date) {
    log.info("Change time according abonent's time zone:");
    log.info("  Abonent send date = " + date);
    final String timezone = Timezones.getTimezoneByAbonent(abonent);
    log.info("  Timezone = " + timezone);

    // Find time difference
    final long d1 = TimeZone.getTimeZone(timezone).getOffset(date.getTime());
    final long d2 = TimeZone.getDefault().getOffset(date.getTime());

    // Calculate SME time by abonent time
    Calendar calend = Calendar.getInstance();
    calend.setTimeInMillis(date.getTime() +  d2 - d1);
    log.info("  Sme send date = " + calend.getTime());
    return calend.getTime();
  }

  private void processMessage(final Message message, final CalendarMessage calendarMessage)  {
    log.info("Message parsing ok: send time = " + calendarMessage.getSendDate() + ", message = " + calendarMessage.getMessage());

    try {
      log.info("Trying to put message into messages list...");

      if (messagesList.canAdd(calendarMessage)) {
        messagesList.add(calendarMessage);
        log.info("Message puted into calendar engine messages list");

      } else {
        calendarMessage.save();
        log.info("Message date is out of working interval. Message puted into DB");
      }

      sendResponse(message, Data.ESME_ROK);

    } catch (SQLException e) {
      sendResponse(message, Data.ESME_RSYSERR);
    }
  }
}
