// Decompiled by Jad v1.5.5.3. Copyright 1997-98 Pavel Kouznetsov.
// Jad home page:      http://web.unicom.com.cy/~kpd/jad.html
// Decompiler options: packimports(3) printdflt
// Source File Name:   CommandWriter.java

package ru.novosoft.smsc.admin.protocol;

import org.apache.log4j.Category;

import java.io.IOException;
import java.io.OutputStream;


// Referenced classes of package ru.novosoft.smsc.admin.protocol:
//      Command


public class CommandWriter
{

  public CommandWriter(OutputStream outStream)
  {
    logger = Category.getInstance(getClass().getName());
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
      len[3 - i] = (byte) (length & 0xff);
      length >>= 8;
    }

    out.write(len);
  }

  private OutputStream out = null;
  protected Category logger = null;
}
