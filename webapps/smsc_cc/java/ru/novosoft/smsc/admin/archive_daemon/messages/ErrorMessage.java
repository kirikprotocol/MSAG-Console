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
public class ErrorMessage extends Message {
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
    IOUtils.writeString32(os, error);
  }

  public void receive(InputStream is) throws IOException {
    error = IOUtils.readString32(is);
  }

}
