package ru.novosoft.smsc.admin.archive_daemon.messages;

import ru.novosoft.smsc.util.IOUtils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Служебный класс для работы с ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public class TotalMessage extends Message {
  private long count = 0;

  public TotalMessage() {
    super(SMSC_BYTE_TOTAL_TYPE);
  }

  public TotalMessage(byte type, long count) {
    super(type);
    this.count = count;
  }

  public long getCount() {
    return count;
  }

  public void send(OutputStream os) throws IOException {
    super.send(os);
    IOUtils.writeInt64(os, count);
  }

  public void receive(InputStream is) throws IOException {
    count = IOUtils.readInt64(is);
  }
}
