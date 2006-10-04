package ru.sibinco.calendarsme.engine.secret;

import ru.aurorisoft.smpp.Message;

import java.util.HashMap;
import java.util.Iterator;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

public class SecretRequestParser {

  private final static String SECRET = "(S|s)(E|e)(C|c)(R|r)(E|e)(T|t)";
  private final static String NONE = "(N|n)(O|o)(N|n)(E|e)";
  private final static String SEC = "(S|s)(E|e)(C|c)";

  private final static String ONE_OR_MORE_SPACES = "\\s+";
  private final static String ANY_NONSPACE_STRING_AFTER_SPACE = "\\s+\\S+";
  private final static String ANY_STRING_AFTER_SPACE = "(\\s*|\\s+.+)";
  private final static String ANY_WORD = "\\s*\\S+\\s*";

  private final static String SECRET_ON_REGEX = SECRET  + ANY_NONSPACE_STRING_AFTER_SPACE;
  private final static String SECRET_OFF_REGEX = SECRET + ONE_OR_MORE_SPACES + NONE;
  private final static String SECRET_MESSAGE_REGEX = SEC + ANY_STRING_AFTER_SPACE;

  static ParseResult parseRequest(final String message) throws ParseException, WrongMessageFormatException {
    try {
      // Order is important here
      if (message.matches(SECRET_OFF_REGEX))
        return new ParseResult(ParseResultType.OFF);
      else if (message.matches(SECRET_ON_REGEX))
        return new ParseResult(ParseResultType.ON, getPassword(message));
      else if (message.matches(SECRET_MESSAGE_REGEX))
        return new ParseResult(ParseResultType.MSG, getMessage(message));
      else if (message.matches(ANY_WORD))
        return new ParseResult(ParseResultType.PWD, message);
    } catch (Throwable e) {
      throw new ParseException(e);
    }

    throw new WrongMessageFormatException();
  }

  private static String getMessage(final String message) {
    return message.split(SEC, 2)[1].trim();
  }

  private static String getPassword(final String message) {
    return message.split(SECRET, 2)[1].trim();
  }

  final static class ParseResult {
    private final ParseResultType type;
    private final String message;

    public ParseResult(final ParseResultType type) {
      this(type, null);
    }

    public ParseResult(final ParseResultType type, final String message) {
      this.type = type;
      this.message = message;
    }

    public ParseResultType getType() {
      return type;
    }

    public String getMessage() {
      return message;
    }
  }

  final static class ParseResultType {
    static final ParseResultType ON = new ParseResultType();
    static final ParseResultType OFF = new ParseResultType();
    static final ParseResultType MSG = new ParseResultType();
    static final ParseResultType PWD = new ParseResultType();

    private ParseResultType() {
    }
  }

  final static class ParseException extends Exception {
    public ParseException(Throwable e) {
      super(e);
    }
  }

  final static class WrongMessageFormatException extends Exception {
  }

  public static void main(String[] args)  {
    HashMap hash = new HashMap();
    hash.put("C","C");
    hash.put("A","A");
    hash.put("B","B");

    for (Iterator iter=hash.keySet().iterator(); iter.hasNext();) {
      System.out.println(hash.get(iter.next()));
    }

    try {
      final Message msg = new Message();
      msg.setMessageString("SEC hey&");
      msg.setSourceAddress("12346125436");
      msg.setDestinationAddress("5707");
      final ParseResult res = parseRequest(msg.getMessageString());
      if (res.getType().equals(ParseResultType.ON))
        System.out.println("ON, password= " + res.getMessage());
      else if (res.getType().equals(ParseResultType.OFF))
        System.out.println("OFF");
      else if (res.getType().equals(ParseResultType.MSG))
        System.out.println("MSG: " + res.getMessage());
      else
        System.out.println("PWD: " + res.getMessage());
    } catch (ParseException e) {
      e.printStackTrace();
    } catch (WrongMessageFormatException e) {
      e.printStackTrace();
    }
  }
}
