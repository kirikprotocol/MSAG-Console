/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 6:07:55 PM
 */
package ru.novosoft.smsc.admin.utli;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.protocol.Command;
import ru.novosoft.smsc.admin.protocol.CommandWriter;
import ru.novosoft.smsc.admin.protocol.Response;
import ru.novosoft.smsc.admin.protocol.ResponseReader;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Date;


public class Proxy
{
  public static final byte StatusConnected = 1;
  public static final byte StatusDisconnected = 0;
  public static final byte StatusTimeEnabled = 1;
  public static final byte StatusTimeDisabled = 0;

  protected String host = "localhost";
  protected int port = 1024;
  protected Category logger = Category.getInstance(this.getClass().getName());
  private byte status = StatusDisconnected;
  private byte statusTime = StatusTimeEnabled;
  private Socket socket = null;
  private OutputStream out;
  private InputStream in;
  private CommandWriter writer;
  private ResponseReader reader;
  private Date timeConnect = new Date(0);

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

  protected void setPort(int port)
  {
    this.port = port;
  }

  public byte getStatus()
  {
    return status;
  }

  public byte getStatusTime()
  {
    return statusTime;
  }

  public synchronized Response runCommand(Command command)
          throws AdminException
  {
    logger.debug("runCommand(@" + command.getClass().getName() + ")");
    logger.debug("  status = " + getStatus()
            + " (" + (getStatus() != StatusConnected ? "disconnected" : "connected") + ")");
    if (getStatus() != StatusConnected)
      connect(host, port);

    try
    {
      logger.debug("write command " + command);
      writer.write(command);
      logger.debug("reading response");
      return reader.read();
    }
    catch (IOException e)
    {
      try {
        reconnect();
        writer.write(command);
        return reader.read();
      } catch (IOException e1) {
        disconnect();
        throw new AdminException(e1.getMessage());
      }
    }
    catch (Exception e)
    {
     throw new AdminException(e.getMessage());        
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
          throws AdminException
  {
    logger.debug("reconnect()");
    disconnect();
    connect(host, port);
  }

  protected void connect(String host, int port)
          throws AdminException
  {
    logger.debug("connect()");
    if (status == StatusConnected) {
      final String err = "Already connected";
      logger.warn(err);
      throw new AdminException(err);
    }

    Date current = new Date();
    long interval = current.getTime() - timeConnect.getTime();
    statusTime = (interval > 60000) ? StatusTimeEnabled:StatusTimeDisabled;
    final String warning = "Couldn't connect to \"" + host + ':' + port + '"';
    if (statusTime != StatusTimeEnabled) {
      logger.debug(warning + " timeout");
      throw new AdminException(warning + " timeout");
    }
    timeConnect = current;

    this.host = host; this.port = port;
    try {
      logger.debug("connecting to \"" + host + ':' + port + "\" ...");
      socket = new Socket(host, port);
      socket.setSoTimeout(180000);
      out = socket.getOutputStream();
      in = socket.getInputStream();
      writer = new CommandWriter(out);
      reader = new ResponseReader(in);
      status = StatusConnected;
      logger.debug("connected to \"" + host + ':' + port + "'");
    } catch (IOException e) {
      final String err = warning + ", nested: "+e.getMessage();
      logger.warn(err);
      throw new AdminException(err);
    }

  }

}
