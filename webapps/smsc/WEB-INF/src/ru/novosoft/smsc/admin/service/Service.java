/*
 * Created by igork
 * Date: Feb 28, 2002
 * Time: 12:35:08 PM
 */
package ru.novosoft.smsc.admin.service;

import org.apache.log4j.Category;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.protocol.Command;
import ru.novosoft.smsc.admin.service.protocol.CommandWriter;
import ru.novosoft.smsc.admin.service.protocol.Response;
import ru.novosoft.smsc.admin.service.protocol.ResponseReader;

public class Service
{
  public Service(String host, int port)
  {
    connect(host, port);
  }

  public void reconnect()
  {
    if (socket != null) {
      try {
        socket.close();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
    connect(host, port);
  }

  public Response runCommand(Command command)
          throws AdminException
  {
    try {
      writer.write(command);
      return reader.read();
    } catch (IOException e) {

      throw new AdminException(e.getMessage());
    }
  }

  private void connect(String host, int port)
  {
    this.host = host;
    this.port = port;
    try {
      socket = new Socket(host, port);
      out = socket.getOutputStream();
      in = socket.getInputStream();
      writer = new CommandWriter(out);
      reader = new ResponseReader(in);
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  private Socket socket = null;
  private OutputStream out;
  private InputStream in;
  private CommandWriter writer;
  private ResponseReader reader;
  private String host = "localhost";
  private int port = 1024;
  private Category logger = Category.getInstance(this.getClass().getName());
}
