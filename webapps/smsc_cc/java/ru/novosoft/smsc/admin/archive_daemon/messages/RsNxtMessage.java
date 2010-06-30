package ru.novosoft.smsc.admin.archive_daemon.messages;

import ru.novosoft.smsc.util.IOUtils;

import java.io.IOException;
import java.io.OutputStream;

/**
 * Служебный класс для работы с ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public class RsNxtMessage extends Message {
  private int next = 1;

  public RsNxtMessage() {
    super(SMSC_BYTE_RSNXT_TYPE);
  }

  public RsNxtMessage(int next) {
    super(SMSC_BYTE_RSNXT_TYPE);
    this.next = next;
  }

  public void send(OutputStream os) throws IOException {
    super.send(os);
    IOUtils.writeUInt32(os, next);
  }

}