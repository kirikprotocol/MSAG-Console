package mobi.eyeline.smsquiz.distribution.smscconsole;

import org.apache.log4j.Logger;


/**
 * author: alkhal
 */

public class ConsoleConnection {

  private static final Logger logger = Logger.getLogger(ConsoleConnection.class);

  private ConsoleSender consoleSender;

  boolean assigned = false;

  private long lastUsage = System.currentTimeMillis();

  private String name;

  ConsoleConnection(String name, String login, String password, String host, int port) {
    this.name = name;
    consoleSender = new ConsoleSenderImpl(login, password, host, port);
  }

  public ConsoleResponse sendCommand(String command) throws ConsoleException {
    lastUsage = System.currentTimeMillis();
    consoleSender.connect();
    if (logger.isDebugEnabled()) {
      logger.debug("Sending command: " + command + " via: " + this);
    }
    return consoleSender.sendCommand(command);
  }

  void setAssigned(boolean assigned) {
    lastUsage = System.currentTimeMillis();
    this.assigned = assigned;
  }

  boolean isAssigned() {
    return assigned;
  }

  public void close() {
    if (logger.isInfoEnabled()) {
      logger.info("Close conn: " + this);
    }
    lastUsage = System.currentTimeMillis();
    assigned = false;
  }

  void disconnect() {
    if (logger.isInfoEnabled()) {
      logger.info("Disconnect conn: " + this);
    }
    consoleSender.disconnect();
    close();
  }

  long getLastUsage() {
    return lastUsage;
  }

  boolean isConnected() {
    return consoleSender.isConnected();
  }

  public String getName() {
    return name;
  }

  @Override
  public String toString() {
    return "ConsoleConnection{" +
        "name='" + name + '\'' +
        ", lastUsage=" + lastUsage +
        ", assigned=" + assigned +
        '}';
  }
}
