/*
 * Created by igork
 * Date: Feb 26, 2002
 * Time: 6:47:05 PM
 */
package ru.novosoft.smsc.admin.service.protocol;

import java.io.IOException;
import java.io.OutputStream;

public class CommandWriter
{
  private OutputStream out;

  public CommandWriter(OutputStream outStream)
  {
    out = outStream;
  }

  public void write(Command command)
          throws IOException
  {
    String txt = command.getText();
    writeLength(txt.length());
    try {
      out.write(txt.getBytes());
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  protected void writeLength(int length)
  {
    byte len[] = new byte[4];
    for (int i = 0; i < 4; i++) {
      len[3 - i] = (byte) (length & 0xFF);
      length >>= 8;
    }
    try {
      out.write(len);
    } catch (IOException e) {
      e.printStackTrace();
    }
  }
}
