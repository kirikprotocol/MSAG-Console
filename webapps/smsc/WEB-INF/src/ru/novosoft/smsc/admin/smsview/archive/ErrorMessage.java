package ru.novosoft.smsc.admin.smsview.archive;

import java.io.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.01.2004
 * Time: 20:57:43
 * To change this template use Options | File Templates.
 */
public class ErrorMessage extends Message
{
  private String error = null;

  public ErrorMessage() {
    super(Message.SMSC_BYTE_ERROR_TYPE);
  }
  public ErrorMessage(String error) {
    super(Message.SMSC_BYTE_ERROR_TYPE);
    this.error = error;
  }
  public String getError() {
    return error;
  }
  public void send(OutputStream os) throws IOException {
    super.send(os);
    Message.writeString32(os, error);
  }
  public void receive(InputStream is) throws IOException {
    error = Message.readString32(is);
  }

}
