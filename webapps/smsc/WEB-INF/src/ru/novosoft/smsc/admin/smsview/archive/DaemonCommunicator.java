package ru.novosoft.smsc.admin.smsview.archive;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 27.01.2004
 * Time: 15:08:03
 * To change this template use Options | File Templates.
 */
public class DaemonCommunicator
{
  private InputStream is = null;
  private OutputStream os = null;

  public DaemonCommunicator(InputStream is, OutputStream os)
  {
    this.is = is;
    this.os = os;
  }

  public void send(Message message) throws IOException
  {
    if (message == null) return;
    message.send(os);
  }

  public Message receive() throws IOException
  {
    byte type = (byte) Message.readUInt8(is);
    Message message = Message.create(type);
    if (message != null) message.receive(is);
    return message;
  }
}
