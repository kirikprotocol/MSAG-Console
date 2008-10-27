package mobi.eyeline.smsquiz.distribution.smscconsole;

/**
 * author: alkhal
 */
public interface ConsoleSender {
  public void connect() throws SmscConsoleException;

  public void disconnect();

  public SmscConsoleResponse sendCommand(String command) throws SmscConsoleException;

}
