/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 6:07:55 PM
 */
package ru.novosoft.smsc.admin.utli;

import org.apache.log4j.Category;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.protocol.Command;
import ru.novosoft.smsc.admin.protocol.CommandWriter;
import ru.novosoft.smsc.admin.protocol.Response;
import ru.novosoft.smsc.admin.protocol.ResponseReader;

public class Proxy
{
  public final byte StatusConnected = 1;
  public final byte StatusDisconnected = 0;

  protected String host = "localhost";
  protected int port = 1024;
  protected Category logger = Category.getInstance(this.getClass().getName());
  private byte status = StatusDisconnected;
  private Socket socket = null;
  private OutputStream out;
  private InputStream in;
  private CommandWriter writer;
  private ResponseReader reader;

  protected Proxy(String host, int port)
          throws AdminException
  {
    logger.debug("Proxy(" + host + ", " + port + ")");
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

  public Response runCommand(Command command)
          throws AdminException
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
        status = StatusDisconnected;
        throw new AdminException(e1.getMessage());
      }
    }
  }

  protected void reconnect()
          throws AdminException
  {
    logger.debug("reconnect()");
    status = StatusDisconnected;
    if (socket != null) {
      try {
        socket.close();
        socket = null;
      } catch (IOException e) {
        logger.debug("Exception on closeSocket", e);
      }
    }
    connect(host, port);
  }

  protected void connect(String host, int port)
          throws AdminException
  {
    logger.debug("connect to \"" + host + ':' + port + '"');
    if (status == StatusDisconnected) {
      this.host = host;
      this.port = port;
      try {
        socket = new Socket(host, port);
        out = socket.getOutputStream();
        in = socket.getInputStream();
        writer = new CommandWriter(out);
        reader = new ResponseReader(in);
        status = StatusConnected;
      } catch (IOException e) {
        logger.warn("Couldn't connect", e);
        throw new AdminException("Couldn't connect to \"" + host + ':' + port + "\", nested: " + e.getMessage());
      }
    } else {
      throw new AdminException("Already connected");
    }
  }
}
