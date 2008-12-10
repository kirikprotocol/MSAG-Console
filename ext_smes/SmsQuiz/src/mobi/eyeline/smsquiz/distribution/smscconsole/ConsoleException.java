package mobi.eyeline.smsquiz.distribution.smscconsole;

/**
 * author: alkhal
 */
public class ConsoleException extends Exception {

  public ConsoleException() {
  }

  public ConsoleException(String message) {
    super(message);
  }

  public ConsoleException(String message, Throwable cause) {
    super(message, cause);
  }

  public ConsoleException(Throwable cause) {
    super(cause);
  }
}
