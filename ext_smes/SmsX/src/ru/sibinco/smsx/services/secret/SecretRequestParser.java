package ru.sibinco.smsx.services.secret;

import ru.aurorisoft.smpp.Message;

import java.util.HashMap;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: Aug 2, 2006
 */

final class SecretRequestParser {

  private final static String SECRET = "(S|s)(E|e|\u0415|\u0435)(C|c|\u0421|\u0441)(R|r)(E|e|\u0415|\u0435)(T|t|\u0422|\u0442)";
  private static final String P = "(P|p|\u0420|\u0440|\u041F|\u043F)";
  private static final String SECRET_ON = "(" + SECRET + "|" + P + ")";
  private final static String NONE = "(N|n)(O|o)(N|n)(E|e|\u0415|\u0435)";
  private final static String SEC = "(((S|s)(E|e|\u0415|\u0435)(C|c|\u0421|\u0441))|#)";
  private final static String SMS = "(S|s)(M|m|\u041C)(S|s)";

  private final static String ONE_OR_MORE_SPACES = "\\s+";
  private final static String ANY_NONSPACE_STRING_AFTER_SPACE = "\\s+\\S+";
  private final static String ANY_STRING_AFTER_SPACE = "(\\s*|\\s+.+)";
//  private final static String ANY_WORD = "\\s*\\S+\\s*";

  private final static String SECRET_ON_REGEX = SECRET_ON  + ANY_NONSPACE_STRING_AFTER_SPACE;
  private final static String SECRET_CHANGE_PASSWORD_REGEX = SECRET_ON + ANY_NONSPACE_STRING_AFTER_SPACE + ANY_NONSPACE_STRING_AFTER_SPACE;
  private final static String SECRET_OFF_REGEX = "(" + SECRET + ONE_OR_MORE_SPACES + NONE + "|-" + P + ")";
  private final static String SECRET_MESSAGE_REGEX = SEC + ANY_STRING_AFTER_SPACE;
  private final static String SECRET_GET_REGEX = SMS + ANY_STRING_AFTER_SPACE;

  static ParseResult parseRequest(final String message) throws ParseException, WrongMessageFormatException, WrongPasswordException {
    try {
      // Order is important here
      if (message.matches(SECRET_OFF_REGEX))
        return new ParseResult(ParseResultType.OFF);
      else if (message.matches(SECRET_CHANGE_PASSWORD_REGEX))
        return new ParseResult(ParseResultType.CHANGE_PWD, getPasswordInSecret(message)); // There in password will be 2 words: old pwd and new pwd
      else if (message.matches(SECRET_ON_REGEX))
        return new ParseResult(ParseResultType.ON, getPasswordInSecret(message));
      else if (message.matches(SECRET_MESSAGE_REGEX))
        return new ParseResult(ParseResultType.MSG, getMessage(message));
      else if (message.matches(SECRET_GET_REGEX))
        return new ParseResult(ParseResultType.PWD, getPasswordInGet(message));

    } catch (Throwable e) {
      throw new ParseException(e);
    }

    if (message.matches(SECRET_ON + ".*") || message.matches(SMS + ".*"))
        throw new WrongPasswordException();

    throw new WrongMessageFormatException();
  }

  private static String removePrefix(final String message, final String prefixPattern) {
    final Matcher matcher = Pattern.compile(prefixPattern).matcher(message);
    matcher.find();
    return message.substring(matcher.end());
  }

  private static String getMessage(final String message) {
    return removePrefix(message, SEC);
//    return message.split(SEC, 2)[1].trim();
  }

  private static String getPasswordInSecret(final String message) {
    return removePrefix(message, SECRET_ON);
//    return message.split(SECRET_ON, 2)[1].trim();
  }

  private static String getPasswordInGet(final String message) {
    return removePrefix(message, SMS);
//    return message.split(SMS, 2)[1].trim();
  }

  final static class ParseResult {
    private final ParseResultType type;
    private final String message;

    public ParseResult(final ParseResultType type) {
      this(type, null);
    }

    public ParseResult(final ParseResultType type, final String message) {
      this.type = type;
      this.message = message.trim();
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
    static final ParseResultType CHANGE_PWD = new ParseResultType();

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

  final static class WrongPasswordException extends Exception {
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
    } catch (WrongPasswordException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    }
  }
}
