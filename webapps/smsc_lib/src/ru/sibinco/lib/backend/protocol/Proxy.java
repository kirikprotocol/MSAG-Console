package ru.sibinco.lib.backend.protocol;

import org.apache.log4j.Logger;
import ru.sibinco.lib.SibincoException;

import java.io.*;
import java.net.Socket;


/**
 * Created by igork
 * Date: 25.05.2004
 * Time: 16:03:21
 */
public class Proxy
{
  public static final byte StatusConnected = 1;
  public static final byte StatusDisconnected = 0;

  protected String host = "localhost";
  protected int port = 1024;
  protected Logger logger = Logger.getLogger(this.getClass());
  private byte status = StatusDisconnected;
  private Socket socket = null;
  private OutputStream out;
  private InputStream in;
  private CommandWriter writer;
  private ResponseReader reader;

  protected Proxy(String host, int port)
  {
    this.host = host;
    this.port = port;
    status = StatusDisconnected;
  }

  public String getHost()
  {
    return host;
  }

  public int getPort()
  {
    return port;
  }

  public byte getStatus()
  {
    return status;
  }

  public synchronized Response runCommand(Command command)
      throws SibincoException
  {
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
    } catch (IOException e) {
      try {
        reconnect();
        writer.write(command);
        return reader.read();
      } catch (IOException e1) {
        disconnect();
        throw new SibincoException("Couldn't write command or read response", e1);
      }
    }
  }

  public void disconnect()
  {
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
      throws SibincoException
  {
    logger.debug("reconnect()");
    disconnect();
    connect(host, port);
  }

  protected void connect(String host, int port)
      throws SibincoException
  {
    logger.debug("connect to \"" + host + ':' + port + '"');
    if (status == StatusDisconnected) {
      this.host = host;
      this.port = port;
      try {
        socket = new Socket(host, port);
        socket.setSoTimeout(180000);
        out = socket.getOutputStream();
        in = socket.getInputStream();
        writer = new CommandWriter(out);
        reader = new ResponseReader(in);
        status = StatusConnected;
      } catch (IOException e) {
        logger.warn("Couldn't connect to \"" + host + ':' + port + "\", nested: " + e.getMessage());
        throw new SibincoException("Couldn't connect to \"" + host + ':' + port + "\", nested: " + e.getMessage());
      }
    } else {
      throw new SibincoException("Already connected");
    }
  }
}
