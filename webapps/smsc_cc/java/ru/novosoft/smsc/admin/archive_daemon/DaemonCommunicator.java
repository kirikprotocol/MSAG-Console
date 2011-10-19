package ru.novosoft.smsc.admin.archive_daemon;

import org.apache.log4j.Logger;
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

  private static final Logger log = Logger.getLogger(DaemonCommunicator.class);

  private InputStream is;
  private OutputStream os;

  DaemonCommunicator(InputStream is, OutputStream os) {
    this.is = is;
    this.os = os;
  }

  void send(Message message) throws IOException {
    if (message == null) return;
    if (log.isDebugEnabled())
      log.debug("Sending message: " + message);
    message.send(os);
    if (log.isDebugEnabled())
      log.debug("Message sent.");
  }

  Message receive() throws IOException {
    byte type = (byte) IOUtils.readUInt8(is);
    Message message = Message.create(type);
    log.debug("Receiving message type=" + type);
    if (message != null) message.receive(is);
    if (log.isDebugEnabled())
      log.debug("Message received: " + message);
    return message;
  }
}
