package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.sme.smpp.IncomingObject;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsc.utils.timezones.SmscTimezone;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesListException;
import ru.sibinco.smsx.Context;
import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarSendMessageCmd;
import ru.sibinco.smsx.engine.service.calendar.commands.CalendarHandleReceiptCmd;

import java.util.Calendar;
import java.util.Date;
import java.util.Properties;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 14.05.2008
 */

public class CalendarSMPPService extends AbstractSMPPService {

  private static final Category log = Category.getInstance("CALENDAR SMPP");

  private final static Pattern ONE_OR_MORE_SPACES = Pattern.compile("\\s+");
  private final static Pattern ZERO_OR_MORE_SPACES = Pattern.compile("\\s*");
  private final static Pattern ONE_OR_TWO_DIGITS = Pattern.compile("\\d{1,2}");
  private final static Pattern ONE_TO_FOUR_DIGITS = Pattern.compile("\\d{1,4}");
  private final static Pattern DOT = Pattern.compile("\\.");
  private final static Pattern COLON = Pattern.compile(":");

  private final static Pattern DATE = Pattern.compile(ONE_OR_TWO_DIGITS.pattern() + ZERO_OR_MORE_SPACES.pattern() + DOT.pattern() +
                                                      ZERO_OR_MORE_SPACES.pattern() + ONE_OR_TWO_DIGITS.pattern() + ZERO_OR_MORE_SPACES.pattern() +
                                                      DOT.pattern() + ZERO_OR_MORE_SPACES.pattern() + ONE_TO_FOUR_DIGITS.pattern());

  private final static Pattern DATE_TIME_MIN = Pattern.compile(DATE.pattern() + ONE_OR_MORE_SPACES.pattern() + ONE_OR_TWO_DIGITS.pattern() +
                                                               ZERO_OR_MORE_SPACES.pattern() + COLON.pattern() + ZERO_OR_MORE_SPACES.pattern() +
                                                               ONE_OR_TWO_DIGITS.pattern());

  private final static Pattern DATE_TIME_SEC = Pattern.compile(DATE_TIME_MIN.pattern() + ZERO_OR_MORE_SPACES.pattern() + COLON.pattern() +
                                                               ZERO_OR_MORE_SPACES.pattern() + ONE_OR_TWO_DIGITS.pattern());

  private String msgSendDateIsWrong;
  private String serviceAddress;

  protected void init(Properties properties) throws SMPPServiceException {
    super.init(properties);
    msgSendDateIsWrong = properties.getProperty("send.date.is.wrong");
    if (msgSendDateIsWrong == null)
      throw new SMPPServiceException("Can't find property 'send.date.is.wrong'");

    serviceAddress = properties.getProperty("service.address");
    if (serviceAddress == null)
      throw new SMPPServiceException("Can't find property 'service.address'");
  }

  public boolean serve(SMPPRequest smppRequest) {

    String type = smppRequest.getParameter("type");
    if (type == null) {
      log.error("'type' property is empty in request");
      return false;
    }

    final IncomingObject inObj = smppRequest.getInObj();

    try {

      if (type.equalsIgnoreCase("receipt")) { // Handle receipt

        final long msgId = Long.parseLong(inObj.getMessage().getReceiptedMessageId());
        final boolean delivered = inObj.getMessage().getMessageState() == Message.MSG_STATE_DELIVERED;

        final CalendarHandleReceiptCmd cmd = new CalendarHandleReceiptCmd();
        cmd.setSmppMessageId(msgId);
        cmd.setDelivered(delivered);
        cmd.setUmr(inObj.getMessage().getUserMessageReference());

        if (Services.getInstance().getCalendarService().execute(cmd)) {
          inObj.respond(Data.ESME_ROK);
          return true;
        }               

      } else { // Handle usual message

        final String msg = smppRequest.getParameter("message");
        final String sourceAddress = inObj.getMessage().getSourceAddress();
        final String destinationAddress = inObj.getMessage().getDestinationAddress();

        if (log.isInfoEnabled())
          log.info("Msg srcaddr=" + sourceAddress + "; dstaddr=" + destinationAddress);

        ParseResult result;
        if (type.equalsIgnoreCase("aft")) {
          result = parseATF(smppRequest.getParameter("message"));
        } else if (type.equalsIgnoreCase("at_date")) {
          result = parseAT(sourceAddress, msg, DATE);
        } else if (type.equalsIgnoreCase("at_date_time_min")) {
          result = parseAT(sourceAddress, msg, DATE_TIME_MIN);
        } else if (type.equalsIgnoreCase("at_date_time_sec")) {
          result = parseAT(sourceAddress, msg, DATE_TIME_SEC);
        } else if (type.equalsIgnoreCase("now")) {
          result = new ParseResult(null, msg);
        } else {
          log.error("Unknown msg type parameter: " + type);
          return false;
        }

        if (log.isInfoEnabled())
          log.info("Senddate=" + result.sendDate);

        final CalendarSendMessageCmd cmd = new CalendarSendMessageCmd();
        cmd.setSourceAddress(sourceAddress);
        cmd.setDestinationAddress(destinationAddress);
        cmd.setSendDate(result.sendDate);
        cmd.setMessage(result.message);
        cmd.setDestAddressSubunit(inObj.getMessage().getDestAddrSubunit());
        cmd.setStoreDeliveryStatus(false);
        cmd.setSourceId(AsyncCommand.SOURCE_SMPP);

        try {
          Services.getInstance().getCalendarService().execute(cmd);
          inObj.respond(Data.ESME_ROK);
        } catch (CommandExecutionException e) {
          switch (e.getErrCode()) {
            case CalendarSendMessageCmd.ERR_SYS_ERROR:
              inObj.respond(Data.ESME_RX_P_APPN);
              break;
            case CalendarSendMessageCmd.ERR_WRONG_SEND_DATE:
              inObj.respond(Data.ESME_ROK);
              reply(inObj.getMessage(), serviceAddress, msgSendDateIsWrong);
              sendMessage(sourceAddress, destinationAddress, inObj.getMessage().getConnectionName(), cmd.getMessage(), cmd.getDestAddressSubunit());
              break;
            case CalendarSendMessageCmd.ERR_WRONG_DESTINATION_ADDRESS:
              inObj.respond(Data.ESME_RINVDSTADR);
              break;
            default:
              inObj.respond(Data.ESME_RX_P_APPN);
              log.error("WARNING: Unknown result type in Calendar Handler: " + e.getErrCode());
          }
        }

        return true;
      }

      return false;

    } catch (Throwable e) {
      log.error(e,e);
      return false;
    }
  }

  private static ParseResult parseATF(String str) {
    Matcher matcher = ONE_TO_FOUR_DIGITS.matcher(str);
    matcher.find();
    final String dateStr = str.substring(matcher.start(), matcher.end()).trim();
    final String message = str.substring(matcher.end()).trim();

    final Calendar calendar = Calendar.getInstance();
    calendar.setTime(new Date());
    calendar.set(Calendar.MINUTE, calendar.get(Calendar.MINUTE) + Integer.parseInt(dateStr.trim()));

    return new ParseResult(calendar.getTime(), message);
  }

  private static ParseResult parseAT(final String sourceAbonent, final String str, final Pattern dateRegex) {
    Matcher matcher = dateRegex.matcher(str);
    matcher.find();
    final String dateStr = str.substring(matcher.start(), matcher.end()).trim();
    final String message = str.substring(matcher.end()).trim();

    Date date = null;
    try {
      date = parseDate(dateStr);
      date = changeDateAccordingTimezone(sourceAbonent, date);
    } catch (WrongSendDateException e) {
      date = null;
    }

    return new ParseResult(date, message);
  }

  private static Date changeDateAccordingTimezone(final String abonent, final Date date) {
    if (date == null)
      return null;

    try {
      final SmscTimezone tz = Context.getInstance().getTimezones().getTimezoneByAddress(abonent);

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
