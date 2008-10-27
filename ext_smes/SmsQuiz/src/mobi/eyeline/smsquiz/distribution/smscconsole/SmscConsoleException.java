package mobi.eyeline.smsquiz.distribution.smscconsole;

/**
 * author: alkhal
 */
public class SmscConsoleException extends Exception{

 public SmscConsoleException() {
  }

  public SmscConsoleException(String message) {
    super(message);
  }

  public SmscConsoleException(String message, Throwable cause) {
    super(message, cause);
  }

  public SmscConsoleException(Throwable cause) {
    super(cause);
  }
}
