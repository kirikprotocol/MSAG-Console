package ru.novosoft.smsc.admin.smsview.archive;

import java.io.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.01.2004
 * Time: 20:21:37
 * To change this template use Options | File Templates.
 */
public class TotalMessage extends Message
{
  private long count = 0;

  public TotalMessage() {
    super(Message.SMSC_BYTE_TOTAL_TYPE);
  }
  public TotalMessage(byte type, long count) {
    super(type); this.count = count;
  }

  public long getCount() {
    return count;
  }
  public void send(OutputStream os) throws IOException {
    super.send(os);
    Message.writeInt64(os, count);
  }
  public void receive(InputStream is) throws IOException {
    count = Message.readInt64(is);
  }
}
