package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.archive_daemon.messages.Message;
import ru.novosoft.smsc.util.IOUtils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Служебный класс для отправки сообщений ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
class DaemonCommunicator {

  private InputStream is;
  private OutputStream os;

  DaemonCommunicator(InputStream is, OutputStream os) {
    this.is = is;
    this.os = os;
  }

  void send(Message message) throws IOException {
    if (message == null) return;
    message.send(os);
  }

  Message receive() throws IOException {
    byte type = (byte) IOUtils.readUInt8(is);
    Message message = Message.create(type);
    if (message != null) message.receive(is);
    return message;
  }
}