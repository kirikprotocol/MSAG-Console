package ru.novosoft.smsc.admin.smsview.archive;

import java.io.OutputStream;
import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.01.2004
 * Time: 21:08:28
 * To change this template use Options | File Templates.
 */
public class RsNxtMessage extends Message
{
  private int next = 1;

  public RsNxtMessage() {
    super(Message.SMSC_BYTE_RSNXT_TYPE);
  }
  public RsNxtMessage(int next) {
    super(Message.SMSC_BYTE_RSNXT_TYPE);
    this.next = next;
  }

  public void send(OutputStream os) throws IOException {
    super.send(os);
    Message.writeUInt32(os, next);
  }

}
