package ru.novosoft.smsc.admin.protocol;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;

import java.io.IOException;
import java.io.InputStream;


class ResponseReader {

  private InputStream in;
  private Category logger = Category.getInstance(ResponseReader.class);

  public ResponseReader(InputStream inStream) {
    in = inStream;
  }

  public Response read() throws IOException, AdminException {
    int length = readLength();
    byte buffer[] = new byte[length];
    for (int readed = 0; readed < length;) {
      int readedNow = 0;
      try {
        readedNow = in.read(buffer, readed, length - readed);
      } catch (IOException e) {
        logger.info("Couldn't read response", e);
        throw e;
      }
      if (readedNow == -1)
        return null;
      readed += readedNow;
    }
    logger.debug("Response:\n" + new String(buffer));
    return new Response(buffer);
  }

  protected int readLength()
      throws IOException {
    int length = 0;
    for (int i = 0; i < 4; i++) {
      int c = 0;
      try {
        c = in.read();
      }
      catch (IOException e) {
        logger.info("Couldn't read response length", e);
        throw e;
      }
      if (c == -1)
        throw new IOException("Couldn't read response length");
      length = (length << 8) + c;
    }
    return length;
  }
}
