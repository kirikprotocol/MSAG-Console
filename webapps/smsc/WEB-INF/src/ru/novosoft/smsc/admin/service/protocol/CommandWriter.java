/*
 * Created by igork
 * Date: Feb 26, 2002
 * Time: 6:47:05 PM
 */
package ru.novosoft.smsc.admin.protocol;

import org.apache.log4j.Category;

import java.io.IOException;
import java.io.OutputStream;

public class CommandWriter
{
  private OutputStream out;
  protected Category logger = Category.getInstance(this.getClass().getName());

  public CommandWriter(OutputStream outStream)
  {
    out = outStream;
  }

  public void write(Command command)
          throws IOException
  {
    logger.debug("writing command " + command);
    String txt = command.getText();
    logger.debug("to write:\n" + txt);
    writeLength(txt.length());
    out.write(txt.getBytes());
  }

  protected void writeLength(int length)
          throws IOException
  {
    byte len[] = new byte[4];
    for (int i = 0; i < 4; i++) {
      len[3 - i] = (byte) (length & 0xFF);
      length >>= 8;
    }
    out.write(len);
  }
}
