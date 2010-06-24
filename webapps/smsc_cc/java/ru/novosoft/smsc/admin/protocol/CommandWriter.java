package ru.novosoft.smsc.admin.protocol;

import org.apache.log4j.Category;

import java.io.IOException;
import java.io.OutputStream;


public class CommandWriter {

  public CommandWriter(OutputStream outStream) {
    logger = Category.getInstance(getClass().getName());
    out = outStream;
  }

  public void write(Command command)
      throws IOException {
    logger.debug("writing command " + command);
    String txt = command.getText();
    logger.debug("to write:\n" + txt);
    writeLength(txt.length());
    out.write(txt.getBytes());
  }

  protected void writeLength(int length)
      throws IOException {
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
