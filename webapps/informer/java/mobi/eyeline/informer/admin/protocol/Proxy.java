package mobi.eyeline.informer.admin.protocol;

import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Category;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Date;


/**
 * Клиент для отправки команд
 */
public class Proxy {

  public static final byte StatusConnected = 1;
  public static final byte StatusDisconnected = 0;
  public static final byte StatusTimeEnabled = 1;
  public static final byte StatusTimeDisabled = 0;

  protected String host = "localhost";
  protected int port = 1024;
  protected final Category logger = Category.getInstance(this.getClass().getName());
  private byte status = StatusDisconnected;
  private byte statusTime = StatusTimeEnabled;
  private Socket socket = null;
  private CommandWriter writer;
  private ResponseReader reader;
  private Date timeConnect = new Date(0);
  private boolean timeMode = true;
  private int timeout = 180000;

  public OutputStream out;
  public InputStream in;

  public Proxy(String host, int port) {
    this.host = host;
    this.port = port;
    status = StatusDisconnected;
  }

  public Proxy(String host, int port, int timeout) {
    this.host = host;
    this.port = port;
    this.timeout = timeout;
    status = StatusDisconnected;
  }

  public String getHost() {
    return host;
  }

  public int getPort() {
    return port;
  }

  protected void setPort(int port) {
    this.port = port;
  }

  public byte getStatus() {
    return status;
  }

  public byte getStatusTime() {
    return statusTime;
  }

  public synchronized Response runCommand(Command command) throws AdminException {
    logger.debug("runCommand(@" + command.getClass().getName() + ")");
    logger.debug("  status = " + getStatus()
        + " (" + (getStatus() != StatusConnected ? "disconnected" : "connected") + ")");
    if (getStatus() != StatusConnected)
      connect(host, port);

    try {
      logger.debug("write command " + command);
      writer.write(command);
      logger.debug("reading response");
      return reader.read();
    }
    catch (IOException e) {
      disconnect();
      throw new ProtocolException("connection_error", e);
    }

  }

  public void disconnect() {
    logger.debug("disconnect()");
    status = StatusDisconnected;
    if (socket != null) {
      try {
        socket.close();
        socket = null;
      } catch (IOException e) {
        logger.debug("Exception on closeSocket", e);
      }
    }
  }

  protected void reconnect()
      throws AdminException {
    logger.debug("reconnect()");
    disconnect();
    connect(host, port);
  }

  protected void connect(String host, int port) throws AdminException {
    logger.debug("connect()");
    if (status == StatusConnected) {
      final String err = "Already connected";
      logger.warn(err);
      return;
    }

    final String warning = "Couldn't connect to \"" + host + ':' + port + '"';

    if (timeMode) {
      final Date current = new Date();
      final long interval = current.getTime() - timeConnect.getTime();
      statusTime = (interval > timeout) ? StatusTimeEnabled : StatusTimeDisabled;
      if (statusTime != StatusTimeEnabled) {
        logger.debug(warning + " timeout");
        throw new ProtocolException("timeout");
      }
      timeConnect = current;
    }

    this.host = host;
    this.port = port;
    try {
      logger.debug("connecting to \"" + host + ':' + port + "\" ...");
      socket = new Socket(host, port);
      if (timeMode)
        socket.setSoTimeout(timeout * 3);
      out = socket.getOutputStream();
      in = socket.getInputStream();
      writer = new CommandWriter(out);
      reader = new ResponseReader(in);
      status = StatusConnected;
      logger.debug("connected to \"" + host + ':' + port + "'");
    } catch (IOException e) {
      final String err = warning + ", nested: " + e.getMessage();
      logger.warn(err);
      throw new ProtocolException("connection_error", e);
    }

  }

  public void setTimeMode(boolean timeMode) {
    this.timeMode = timeMode;
  }
}
