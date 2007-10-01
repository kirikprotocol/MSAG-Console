package ru.sibinco.smsx.network.smscconsole;

/**
 * User: artem
 * Date: 17.07.2007
 */

/**
 * Client for SMSC console
 */
public interface SmscConsoleClient {
  /**
   * Connect to SMSC console
   * @throws SmscConsoleClientException
   */
  public void connect() throws SmscConsoleClientException;

  /**
   * Close connection with SMSC console
   */
  public void close();

  /**
   * Send command to SMSC console
   * @param command command string
   * @return console response
   * @throws SmscConsoleClientException
   */
  public SmscConsoleResponse sendCommand(String command) throws SmscConsoleClientException;
}
