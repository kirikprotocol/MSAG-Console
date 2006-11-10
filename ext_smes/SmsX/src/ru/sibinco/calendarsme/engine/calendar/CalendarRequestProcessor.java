/**
 * User: artem
 * Date: Jul 28, 2006
 */

package ru.sibinco.calendarsme.engine.calendar;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.calendarsme.InitializationException;
import ru.sibinco.calendarsme.engine.timezones.Timezones;
import ru.sibinco.calendarsme.utils.ConnectionPool;
import ru.sibinco.calendarsme.utils.Utils;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Calendar;
import java.util.Date;
import java.util.Properties;
import java.util.TimeZone;

final class CalendarRequestProcessor {
  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(CalendarRequestProcessor.class);
  private static final String NAME = "CalendarRequestProcessor";

  private final String insertMessageSQL;

  private final CalendarMessagesList messagesList;
  private final Multiplexor multiplexor;

  public CalendarRequestProcessor(final Properties config, final CalendarMessagesList messagesList, final Multiplexor multiplexor) {
    if (config == null)
      throw new InitializationException(NAME + ": config is not specified");
    if (messagesList == null)
      throw new InitializationException(NAME + ": messages list not specified");


    this.insertMessageSQL = Utils.loadString(config, "request.processor.insert.message.sql");
    this.messagesList = messagesList;
    this.multiplexor = multiplexor;
  }

  public boolean processRequest(final Message message)  {
    if (message == null)
      return false;
    try {
      final CalendarRequestParser.ParseResult parseResult = CalendarRequestParser.parseRequest(message.getMessageString());

      Log.info("=====================================================================================");
      Log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

      final Date sendDate = (parseResult.getType() == CalendarRequestParser.ATF_REQUEST) ? parseResult.getDate() :
        changeDateAccordingTimezone(message.getSourceAddress(), parseResult.getDate());

      processMessage(new CalendarMessage(message.getSourceAddress(), message.getDestinationAddress(), sendDate, parseResult.getMessage()));

    } catch (CalendarRequestParser.WrongMessageFormatException e) {
      return false;

    } catch (CalendarRequestParser.ParseException e) {
      Log.error("", e);
      sendResponse(message, Data.ESME_RSYSERR);
    } catch (ProcessingException e) {
      Log.error("", e);
      sendResponse(message, Data.ESME_RSYSERR);
    }
    return true;
  }



  private Date changeDateAccordingTimezone(final String abonent, final Date date) throws ProcessingException {

    Log.info("Change time according abonent's time zone:");
    Log.info("  Abonent send date = " + date);
    final String timezone = Timezones.getTimezoneByAbonent(abonent);
    Log.info("  Timezone = " + timezone);

    // Find time difference
    final long d1 = TimeZone.getTimeZone(timezone).getOffset(date.getTime());
    final long d2 = TimeZone.getDefault().getOffset(date.getTime());

    // Calculate SME time by abonent time
    Calendar calend = Calendar.getInstance();
    calend.setTimeInMillis(date.getTime() +  d2 - d1);
    Log.info("  Sme send date = " + calend.getTime());
    return calend.getTime();
  }

  private void processMessage(final CalendarMessage calendarMessage) throws ProcessingException {
    Log.info("Message parsing ok: send time = " + calendarMessage.getSendDate() + ", message = " + calendarMessage.getMessage());

    Log.info("Trying to put message into messages list...");
    if (messagesList.add(calendarMessage))
      Log.info("Message puted into calendar engine messages list");
    else {
      insertMessageToDB(calendarMessage);
      Log.info("Message date is out of working interval. Message puted into DB");
    }
  }

  private void insertMessageToDB(final CalendarMessage calendarMessage) throws ProcessingException{
    Connection conn = null;
    PreparedStatement ps = null;

    try {
      conn = ConnectionPool.getConnection();
      ps = conn.prepareStatement(insertMessageSQL);

      ps.setString(1, calendarMessage.getSource());
      ps.setString(2, calendarMessage.getDest());
      ps.setTimestamp(3, calendarMessage.getSendDate());
      ps.setString(4, calendarMessage.getMessage());

      ps.executeUpdate();

    } catch (SQLException e) {
      Log.error("Can't insert message: ", e);
      throw new ProcessingException();
    } finally {
      close(conn, ps);
    }
  }

  private static void close(final Connection conn, final PreparedStatement ps) {
    try {
      if (ps != null)
        ps.close();
      if (conn != null) {
        conn.commit();
        conn.close();
      }
    } catch (SQLException e) {
      Log.error("Can't close: ", e);
    }
  }

  private void sendResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      multiplexor.sendResponse(msg);
      Log.debug(NAME + ": Delivery response sent, address #" + msg.getDestinationAddress() + "; abonent #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      Log.warn("Exception occured sending delivery response.", e);
    }
  }

  /**
   * Exceptions
   */

  public final static class ProcessingException extends Exception {
  }
}
