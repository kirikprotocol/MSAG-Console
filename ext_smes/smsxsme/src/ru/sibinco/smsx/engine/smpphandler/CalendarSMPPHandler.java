package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.utils.config.ConfigException;
import com.eyeline.sme.utils.config.properties.PropertiesConfig;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.sibinco.smsc.utils.timezones.SmscTimezone;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesListException;
import ru.sibinco.smsx.engine.service.ServiceManager;
import ru.sibinco.smsx.engine.service.CommandObserver;
import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarHandleReceiptCmd;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;
import ru.aurorisoft.smpp.Message;

import java.io.File;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 29.06.2007
 */

class CalendarSMPPHandler extends SMPPHandler {

  private static final Category log = Category.getInstance("CALENDAR SMPP");

  private final static Pattern AFT = Pattern.compile("(((A|a|\u0410|\u0430)(F|f)(T|t|\u0422|\u0442))|(M|m|\u041C|\u043C))");
  private static final Pattern AFT_SPACE = Pattern.compile(AFT.pattern() + "\\s+");
  private final static Pattern ONE_OR_MORE_SPACES = Pattern.compile("\\s+");
  private final static Pattern ZERO_OR_MORE_SPACES = Pattern.compile("\\s*");
  private final static Pattern AT = Pattern.compile("((A|a|\u0410|\u0430)(T|t|\u0422|\u0442)|(D|d|\u0414|\u0434))");
  private final static Pattern ONE_OR_TWO_DIGITS = Pattern.compile("\\d{1,2}");
  private final static Pattern ONE_TO_FOUR_DIGITS = Pattern.compile("\\d{1,4}");
  private final static Pattern DOT = Pattern.compile("\\.");
  private final static Pattern COLON = Pattern.compile(":");
  private final static Pattern ANY_STRING_AFTER_SPACE = Pattern.compile("(\\s*|\\s+.*)");
  private final static Pattern ANY_STRING = Pattern.compile(".*");

  // Dates regexes
  private final static Pattern DATE = Pattern.compile(ONE_OR_TWO_DIGITS.pattern() + ZERO_OR_MORE_SPACES.pattern() + DOT.pattern() +
                                                      ZERO_OR_MORE_SPACES.pattern() + ONE_OR_TWO_DIGITS.pattern() + ZERO_OR_MORE_SPACES.pattern() +
                                                      DOT.pattern() + ZERO_OR_MORE_SPACES.pattern() + ONE_TO_FOUR_DIGITS.pattern());

  private final static Pattern DATE_TIME_MIN = Pattern.compile(DATE.pattern() + ONE_OR_MORE_SPACES.pattern() + ONE_OR_TWO_DIGITS.pattern() +
                                                               ZERO_OR_MORE_SPACES.pattern() + COLON.pattern() + ZERO_OR_MORE_SPACES.pattern() +
                                                               ONE_OR_TWO_DIGITS.pattern());

  private final static Pattern DATE_TIME_SEC = Pattern.compile(DATE_TIME_MIN.pattern() + ZERO_OR_MORE_SPACES.pattern() + COLON.pattern() +
                                                               ZERO_OR_MORE_SPACES.pattern() + ONE_OR_TWO_DIGITS.pattern());

  // Messages regexes
  private final static Pattern AFT_REGEX = Pattern.compile(AFT_SPACE.pattern() + ONE_TO_FOUR_DIGITS.pattern() + ANY_STRING_AFTER_SPACE.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
  private final static Pattern AT_REGEX_DATE = Pattern.compile(AT.pattern() + ONE_OR_MORE_SPACES.pattern() + DATE.pattern() + ANY_STRING_AFTER_SPACE.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
  private final static Pattern AT_REGEX_DATE_TIME_MIN = Pattern.compile(AT.pattern() + ONE_OR_MORE_SPACES.pattern() + DATE_TIME_MIN.pattern() + ANY_STRING_AFTER_SPACE.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
  private final static Pattern AT_REGEX_DATE_TIME_SEC = Pattern.compile(AT.pattern() + ONE_OR_MORE_SPACES.pattern() + DATE_TIME_SEC.pattern() + ANY_STRING_AFTER_SPACE.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
  private final static Pattern AT_REGEX_NOW = Pattern.compile(AT.pattern() + ANY_STRING_AFTER_SPACE.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
  private final static Pattern AFT_REGEX_NOW = Pattern.compile(AFT.pattern() + ANY_STRING.pattern(), Pattern.CASE_INSENSITIVE | Pattern.DOTALL);


  // Properties
  private final String msgSendDateIsWrong;
  private final String serviceAddress;

  private final SmscTimezonesList timezonesList;

  CalendarSMPPHandler(String configDir, SmscTimezonesList timezonesList, SMPPMultiplexor multiplexor) {
    super(multiplexor);

    this.timezonesList = timezonesList;

    try {
      final PropertiesConfig config = new PropertiesConfig(new File(configDir, "smpphandlers/calendarhandler.properties"));

      msgSendDateIsWrong = config.getString("send.date.is.wrong");
      serviceAddress = config.getString("service.address");

    } catch (ConfigException e) {
      throw new SMPPHandlerInitializationException(e);
    }
  }

  protected boolean handleInObj(final SMPPTransportObject inObj) {
    final long start = System.currentTimeMillis();

    try {
      if (inObj.getIncomingMessage() != null && inObj.getIncomingMessage().isReceipt()) {

        sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);

        final long msgId = Long.parseLong(inObj.getIncomingMessage().getReceiptedMessageId());
        final boolean delivered = inObj.getIncomingMessage().getMessageState() == Message.MSG_STATE_DELIVERED;

        final CalendarHandleReceiptCmd cmd = new CalendarHandleReceiptCmd();
        cmd.setSmppMessageId(msgId);
        cmd.setDelivered(delivered);

        return ServiceManager.getInstance().getCalendarService().execute(cmd);

      } if (inObj.getIncomingMessage() != null && inObj.getIncomingMessage().getMessageString() != null) {

        final String msg = inObj.getIncomingMessage().getMessageString().trim();
        final String sourceAddress = inObj.getIncomingMessage().getSourceAddress();
        final String destinationAddress = inObj.getIncomingMessage().getDestinationAddress();

        if (log.isInfoEnabled())
          log.info("Msg srcaddr=" + sourceAddress + "; dstaddr=" + destinationAddress);

        // NOTE: Order is important here
        ParseResult result = null;
        if (AFT_REGEX.matcher(msg).matches())
          result = parseATF(msg, ONE_TO_FOUR_DIGITS);
        else if (AT_REGEX_DATE_TIME_SEC.matcher(msg).matches())
          result = parseAT(sourceAddress, msg, DATE_TIME_SEC);
        else if (AT_REGEX_DATE_TIME_MIN.matcher(msg).matches())
          result = parseAT(sourceAddress, msg, DATE_TIME_MIN);
        else if (AT_REGEX_DATE.matcher(msg).matches())
          result = parseAT(sourceAddress, msg, DATE);
        else if (AT_REGEX_NOW.matcher(msg).matches())
          result = parseAT(sourceAddress, msg, null);
        else if (AFT_REGEX_NOW.matcher(msg).matches())
          result = parseATF(msg, null);

        if (result == null) {
          if (log.isInfoEnabled())
            log.info("Msg format is unknown");
          return false;
        }

        if (log.isInfoEnabled())
          log.info("Senddate=" + result.sendDate);

        final CalendarSendMessageCmd cmd = new CalendarSendMessageCmd();
        cmd.setSourceAddress(sourceAddress);
        cmd.setDestinationAddress(destinationAddress);
        cmd.setSendDate(result.sendDate);
        cmd.setMessage(result.message);
        cmd.setDestAddressSubunit(inObj.getIncomingMessage().getDestAddrSubunit());
        cmd.setStoreDeliveryStatus(false);
        cmd.setSourceId(Command.SOURCE_SMPP);
        cmd.addExecutionObserver(new CommandObserver() {
          public void update(Command command) {
            final CalendarSendMessageCmd cmd = (CalendarSendMessageCmd)command;
            switch (cmd.getStatus()) {
              case CalendarSendMessageCmd.STATUS_SUCCESS:
                sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
                break;
              case CalendarSendMessageCmd.STATUS_SYSTEM_ERROR:
                sendResponse(inObj.getIncomingMessage(), Data.ESME_RSYSERR);
                break;
              case CalendarSendMessageCmd.STATUS_WRONG_SEND_DATE:
                sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);
                sendMessage(serviceAddress, sourceAddress, msgSendDateIsWrong);
                sendMessage(sourceAddress, destinationAddress, cmd.getMessage(), cmd.getDestAddressSubunit());
                break;
              case CalendarSendMessageCmd.STATUS_WRONG_DESTINATION_ADDRESS:
                sendResponse(inObj.getIncomingMessage(), Data.ESME_RINVDSTADR);
                break;
              default:
                sendResponse(inObj.getIncomingMessage(), Data.ESME_RSYSERR);
                log.error("WARNING: Unknown result type in Calendar Handler");
            }
          }
        });

        ServiceManager.getInstance().getCalendarService().execute(cmd);

        return true;
      }

      return false;

    } catch (Throwable e) {
      log.error(e,e);
      sendResponse(inObj.getIncomingMessage(), Data.ESME_RSYSERR);
      return true;

    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }

  }

  private static ParseResult parseATF(String str, Pattern dateRegex) {
    Matcher matcher = AFT.matcher(str);
    matcher.find();
    final String dateAndMessage = str.substring(matcher.end());

    if (dateRegex == null)
      return new ParseResult(null, dateAndMessage);

    matcher = ONE_TO_FOUR_DIGITS.matcher(dateAndMessage);
    matcher.find();
    final String dateStr = dateAndMessage.substring(matcher.start(), matcher.end()).trim();
    final String message = dateAndMessage.substring(matcher.end()).trim();

    final Calendar calendar = Calendar.getInstance();
    calendar.setTime(new Date());
    calendar.set(Calendar.MINUTE, calendar.get(Calendar.MINUTE) + Integer.parseInt(dateStr.trim()));

    return new ParseResult(calendar.getTime(), message);
  }

  private ParseResult parseAT(final String sourceAbonent, final String str, final Pattern dateRegex) {
    Matcher matcher = AT.matcher(str);
    matcher.find();

    final String dateAndMessage = str.substring(matcher.end());
    if (dateRegex == null)
      return new ParseResult(null, dateAndMessage);

    matcher = dateRegex.matcher(dateAndMessage);
    matcher.find();
    final String dateStr = dateAndMessage.substring(matcher.start(), matcher.end()).trim();
    final String message = dateAndMessage.substring(matcher.end()).trim();

    Date date = null;
    try {
      date = parseDate(dateStr);
      date = changeDateAccordingTimezone(sourceAbonent, date);
    } catch (WrongSendDateException e) {
      date = null;
    }

    return new ParseResult(date, message);
  }

  private Date changeDateAccordingTimezone(final String abonent, final Date date) {
    if (date == null)
      return null;

    try {
      final SmscTimezone tz = timezonesList.getTimezoneByAddress(abonent);

      if (tz != null) {
        final String timezone = tz.getName();
        if (log.isDebugEnabled())
          log.debug("Found timezone: " + timezone);

        // Find time difference
        final long d1 = TimeZone.getTimeZone(timezone).getOffset(date.getTime());
        final long d2 = TimeZone.getDefault().getOffset(date.getTime());

        // Calculate SME time by abonent time
        Calendar calend = Calendar.getInstance();
        calend.setTimeInMillis(date.getTime() + d2 - d1);
        return calend.getTime();
      } else {
        log.error("Can't get timezone by abonent: " + abonent);
        return null;
      }
    } catch (SmscTimezonesListException e) {
      log.error("Can't get timezone by abonent", e);
      return null;
    }

  }

  private static Date parseDate(final String dateStr) throws WrongSendDateException {
    final String[] strings = DOT.split(dateStr);
    final int day = Integer.parseInt(strings[0].trim());
    final int months = Integer.parseInt(strings[1].trim());
    if (months - 1 > 11)
      throw new WrongSendDateException();

//    final String[] strings1 = strings[2].trim().split(ONE_OR_MORE_SPACES);
    final String[] strings1 = ONE_OR_MORE_SPACES.split(strings[2]);
    final String Y = "2000";
    final String yearStr = strings1[0].length()>=Y.length() ? strings1[0] : Y.substring(0, Y.length() - strings1[0].length()) + strings1[0];
    final int year = Integer.parseInt(yearStr);

    // Check date
    Calendar calend = Calendar.getInstance();
    calend.set(Calendar.MONTH, months - 1);
    calend.set(Calendar.YEAR, year);
    if (day == 0 || day > calend.getActualMaximum(Calendar.DAY_OF_MONTH))
      throw new WrongSendDateException();

    final Calendar calendar = Calendar.getInstance();
    calendar.setTime(new Date());

    if (year == calendar.get(Calendar.YEAR) && months -1 == calendar.get(Calendar.MONTH) && day == calendar.get(Calendar.DAY_OF_MONTH))
      calendar.set(Calendar.MINUTE, calendar.get(Calendar.MINUTE) + 10);


    int hour = calendar.get(Calendar.HOUR_OF_DAY);
    int minute = calendar.get(Calendar.MINUTE);
    int second = calendar.get(Calendar.SECOND);

    if (strings1.length > 1) {
      final String[] strings2 = COLON.split(strings1[1].trim());
      hour = Integer.parseInt(strings2[0]);
      minute = Integer.parseInt(strings2[1]);
      if (strings2.length == 3)
        second = Integer.parseInt(strings2[2]);
      else second = 0;
    }

    if (hour > 23 || minute > 59 || second > 59)
      throw new WrongSendDateException();

    calendar.set(year, months - 1, day, hour, minute, second);

    return calendar.getTime();
  }

  private static class WrongSendDateException extends Exception {
  }

  private static class ParseResult {
    Date sendDate;
    String message;

    private ParseResult(Date sendDate, String message) {
      this.sendDate = sendDate;
      this.message = message;
    }
  }

}
