package ru.sibinco.calendarsme.engine.calendar;

import ru.sibinco.calendarsme.SmeProperties;

import java.util.Calendar;
import java.util.Date;

/**
 * User: artem
 * Date: Jul 27, 2006
 */

final class CalendarRequestParser {

  public static final int AT_REQUEST = 0;
  public static final int ATF_REQUEST = 1;

  // Regex primitives
  private final static String AFT = "(A|a)(F|f)(T|t)";
  private final static String ONE_OR_MORE_SPACES = "\\s+";
  private final static String ZERO_OR_MORE_SPACES = "\\s*";
  private final static String AT = "(A|a)(T|t)";
  private final static String ONE_OR_TWO_DIGITS = "\\d{1,2}";
  private final static String ONE_TO_FOUR_DIGITS = "\\d{1,4}";
  private final static String FOUR_DIGITS = "\\d{4}";
  private final static String DOT = "\\.";
  private final static String COLON = ":";
  private final static String ANY_STRING_AFTER_SPACE = "(\\s*|\\s+.*)";

  // Dates regexes
  private final static String DATE = ONE_OR_TWO_DIGITS + ZERO_OR_MORE_SPACES + DOT + ZERO_OR_MORE_SPACES +
                                     ONE_OR_TWO_DIGITS + ZERO_OR_MORE_SPACES + DOT + ZERO_OR_MORE_SPACES +
                                     FOUR_DIGITS;

  private final static String DATE_TIME_MIN = DATE + ONE_OR_MORE_SPACES + ONE_OR_TWO_DIGITS + ZERO_OR_MORE_SPACES +
                                              COLON + ZERO_OR_MORE_SPACES + ONE_OR_TWO_DIGITS;

  private final static String DATE_TIME_SEC = DATE_TIME_MIN + ZERO_OR_MORE_SPACES + COLON + ZERO_OR_MORE_SPACES +
                                              ONE_OR_TWO_DIGITS;

  // Messages regexes
  private final static String ATF_REGEX = AFT + ONE_OR_MORE_SPACES + ONE_TO_FOUR_DIGITS + ANY_STRING_AFTER_SPACE;
  private final static String AT_REGEX_DATE = AT + ONE_OR_MORE_SPACES + DATE + ANY_STRING_AFTER_SPACE;
  private final static String AT_REGEX_DATE_TIME_MIN = AT + ONE_OR_MORE_SPACES + DATE_TIME_MIN + ANY_STRING_AFTER_SPACE;
  private final static String AT_REGEX_DATE_TIME_SEC = AT + ONE_OR_MORE_SPACES + DATE_TIME_SEC + ANY_STRING_AFTER_SPACE;

  static ParseResult parseRequest(final String message) throws ParseException, WrongMessageFormatException, WrongSendDateException{
    try {
      final String msg = message.trim();

      if (msg.matches(ATF_REGEX))
        return parseATF(msg);
      // Next ifs order is important
      else if (msg.matches(AT_REGEX_DATE_TIME_SEC))
        return parseAT(msg, DATE_TIME_SEC);
      else if (msg.matches(AT_REGEX_DATE_TIME_MIN))
        return parseAT(msg, DATE_TIME_MIN);
      else if (msg.matches(AT_REGEX_DATE))
        return parseAT(msg, DATE);
    } catch (WrongSendDateException e) {
      throw new WrongSendDateException();
    } catch (Throwable e) {
      throw new ParseException(e);
    }
    // Wrong format
    throw new WrongMessageFormatException();
  }

  private static ParseResult parseATF(final String str) {
    final String dateAndMessage = str.split(AFT + ONE_OR_MORE_SPACES, 2)[1].trim() + " "; // Space is neccessary here
    final String message = dateAndMessage.split(ONE_TO_FOUR_DIGITS,2)[1].trim();
    final String dateStr = dateAndMessage.substring(0, dateAndMessage.length() - message.length() - 1);

    final Calendar calendar = Calendar.getInstance();
    calendar.setTime(new Date());
    calendar.set(Calendar.MINUTE, calendar.get(Calendar.MINUTE) + Integer.parseInt(dateStr.trim()));

    return new ParseResult(ATF_REQUEST, message, calendar.getTime());
  }

  private static ParseResult parseAT(final String str, final String dateRegex) throws  WrongSendDateException {
    final String dateAndMessage = str.split(AT + ONE_OR_MORE_SPACES, 2)[1].trim() + " "; // Space is neccessary here
    final String message = dateAndMessage.split(dateRegex)[1].trim();
    final String dateStr = dateAndMessage.substring(0, dateAndMessage.length() - message.length() - 1);
    return new ParseResult(AT_REQUEST, message, parseDate(dateStr));
  }



  private static Date parseDate(final String dateStr) throws  WrongSendDateException {
    final String[] strings = dateStr.split(DOT);
    final int day = Integer.parseInt(strings[0].trim());
    final int months = Integer.parseInt(strings[1].trim());
    if (months - 1 > 11)
      throw new WrongSendDateException();

    final String[] strings1 = strings[2].trim().split(ONE_OR_MORE_SPACES);
    final int year = Integer.parseInt(strings1[0]);

    if (year == 0 || year > SmeProperties.General.CALENDAR_SEND_DATE_MAX_YEAR )
      throw new WrongSendDateException();

    // Check date
    Calendar calend = Calendar.getInstance();
    calend.set(Calendar.MONTH, months - 1);
    calend.set(Calendar.YEAR, year);
    if (day == 0 || day > calend.getActualMaximum(Calendar.DAY_OF_MONTH))
      throw new WrongSendDateException();

    final Calendar calendar = Calendar.getInstance();
    calendar.setTime(new Date());

    int hour = calendar.get(Calendar.HOUR);
    int minute = calendar.get(Calendar.MINUTE);
    int second = calendar.get(Calendar.SECOND);
    if (strings1.length > 1) {
      final String[] strings2 = strings1[1].trim().split(COLON);
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


  /**
   * Parsing tests here
   * @param args
   */
  public static void main(String[] args) {

//    msg.setMessageString("ATF 1 rewt...");
    final String msg = "AT 29.2.2008 good1";
    final ParseResult res;
    try {
      res = parseRequest(msg);
      System.out.println(res.getDate().toString() + " ||| " + res.getMessage());
    } catch (ParseException e) {
      e.printStackTrace();
    } catch (WrongMessageFormatException e) {
      System.out.println("Wrong message format");
    } catch (WrongSendDateException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    }
  }

  public static final class ParseResult {
    private final int type;
    private final String message;
    private final Date date;

    public ParseResult(final int type, final String message, final Date date) {
      this.type = type;
      this.message = message;
      this.date = date;
    }

    public int getType() {
      return type;
    }

    public String getMessage() {
      return message;
    }

    public Date getDate() {
      return date;
    }
  }
  /**
   * Exception in parsing message
   */
  public static final class ParseException extends Exception {
    public ParseException(final Throwable e) {
      super(e);
    }
  }

  /**
   * Wrong or unknown message format
   */
  public static class WrongMessageFormatException extends Exception{
  }

  /**
   * Wrong send date
   */
  public static class WrongSendDateException extends Exception {
  }
}
