package mobi.eyeline.smsquiz.distribution.smscconsole;

/**
 * author: alkhal
 */
public interface ConsoleSender {
  public void connect() throws ConsoleException;

  public void disconnect();

  public ConsoleResponse sendCommand(String command) throws ConsoleException;

  public boolean isConnected();

}
