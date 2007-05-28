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

      if (message.getDestinationAddress().equals(CalendarService.Properties.SERVICE_ADDRESS)) {
        log.error("Destination address in calendar equals to Calendar service address: " + CalendarService.Properties.SERVICE_ADDRESS + ". Sends notification.");
        sendResponse(message, Data.ESME_RX_P_APPN);
        sendMessage(CalendarService.Properties.SERVICE_ADDRESS, message.getSourceAddress(), CalendarService.Properties.WRONG_DESTINATION_ADDRESS);
        return;
      }

      final CalendarRequestParser.ParseResult parseResult = parsedMessage.getParseResult();

      final Date sendDate = (parseResult.getType() == CalendarRequestParser.AFT_REQUEST) ? parseResult.getDate() :
        changeDateAccordingTimezone(message.getSourceAddress(), parseResult.getDate());

      // Check send date
      String errorText = null;
      if (sendDate == null)
        errorText = CalendarService.Properties.SEND_DATE_IS_WRONG;
      else if (sendDate.before(new Date()))
        errorText = CalendarService.Properties.SEND_DATE_IS_IN_THE_PAST;

      if (errorText != null) {
        sendResponse(message, Data.ESME_ROK);
        sendMessage(CalendarService.Properties.SERVICE_ADDRESS, message.getSourceAddress(), errorText);
        sendMessage(message.getSourceAddress(), message.getDestinationAddress(), parsedMessage.getParseResult().getMessage());
        return;
      }

      processMessage(message, new CalendarMessage(message.getSourceAddress(), message.getDestinationAddress(), sendDate, message.getDestAddrSubunit(), parseResult.getMessage()));

    } catch (Exception e) {
      log.error(e);
      sendResponse(message, Data.ESME_RSYSERR);
    }
  }

  private Date changeDateAccordingTimezone(final String abonent, final Date date) {
    if (date == null)
      return null;
    
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
