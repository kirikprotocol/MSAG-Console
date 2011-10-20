package ru.novosoft.smsc.admin.archive_daemon;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.messages.*;
import ru.novosoft.smsc.admin.util.ProgressObserver;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

@SuppressWarnings({"EmptyCatchBlock"})

/**
 * API для простмотра статистики смс
 * @author Aleksandr Khalitov
 */

public class ArchiveDaemon {

  private final static int MAX_SMS_FETCH_SIZE = 5000;

  private static final Logger logger = Logger.getLogger(ArchiveDaemon.class);

  private final ArchiveDaemonManager manager;

  public ArchiveDaemon(ArchiveDaemonManager manager) throws AdminException {
    this.manager = manager;
  }

  private Socket connect() throws AdminException, IOException {
    if (manager.getDaemonOnlineHost() == null)
      throw new ArchiveDaemonException("archive_daemon_offline");
    String host = manager.getSettings().getViewHost();
    int port = manager.getSettings().getViewPort();

    return new Socket(host, port);
  }

  /**
   * Возвращает статистику смс, удовлетворяющую запросу
   *
   * @param query запрос
   * @param observer отслеживание прогресса
   * @return статистика смс
   * @throws AdminException ошибка извлечения статистики
   */
  public SmsSet getSmsSet(ArchiveMessageFilter query, ProgressObserver observer) throws AdminException {
    Socket socket = null;
    InputStream input = null;
    OutputStream output = null;

    SmsSet set = new SmsSet();
    set.setHasMore(false);
    int rowsMaximum = query.getRowsMaximum();
    observer.update(0, rowsMaximum);
    if (rowsMaximum == 0) return set;

    QueryMessage request = new QueryMessage(query);

    try {

      socket = connect();
      input = socket.getInputStream();
      output = new BufferedOutputStream(socket.getOutputStream());

      DaemonCommunicator communicator = new DaemonCommunicator(input, output);
      communicator.send(request);
      int toReceive = rowsMaximum < MAX_SMS_FETCH_SIZE ? rowsMaximum : MAX_SMS_FETCH_SIZE;

      communicator.send(new RsNxtMessage(toReceive));

      Message responce;
      boolean allSelected = false;
      int counter = 0;
      do {
        responce = communicator.receive();
        if (responce == null) throw new ArchiveDaemonException("invalid_response");

        switch (responce.getType()) {
          case Message.SMSC_BYTE_EMPTY_TYPE:
            allSelected = true;
            break;
          case Message.SMSC_BYTE_RSSMS_TYPE:
            set.addRow(((RsSmsMessage) responce).getSms());
            observer.update(++counter<rowsMaximum ? counter : rowsMaximum, rowsMaximum);
            if (--toReceive <= 0) {
              toReceive = rowsMaximum - set.getRowsCount();
              if (toReceive <= 0) {
                set.setHasMore(true);
                communicator.send(new EmptyMessage());
              } else {
                toReceive = (toReceive < MAX_SMS_FETCH_SIZE) ? toReceive : MAX_SMS_FETCH_SIZE;
                communicator.send(new RsNxtMessage(toReceive));
              }
            }
            break;
          case Message.SMSC_BYTE_ERROR_TYPE:
            logger.error("ArchiveDaemon communication error: " + ((ErrorMessage) responce).getError());
            throw new ArchiveDaemonException("error_returned");
          default:
            logger.error("Unknown response type: " + responce.getType());
            throw new ArchiveDaemonException("invalid_response");
        }
      } while (!allSelected);
      observer.update(rowsMaximum, rowsMaximum);

    } catch (IOException exc) {
      throw new ArchiveDaemonException("communication_error", exc);
    } finally {
      close(input, output, socket);
    }
    return set;
  }

  /**
   * Возвращает кол-во смс, удовлетворяющим запросу
   *
   * @param query запрос
   * @return кол-во смс
   * @throws AdminException ошибка извлечения статистики
   */
  public int getSmsCount(ArchiveMessageFilter query) throws AdminException {

    System.out.println("GET SMS COUNT");
    Socket socket = null;
    InputStream input = null;
    OutputStream output = null;

    int smsCount = 0;
    try {
      CountMessage request = new CountMessage(query);

      socket = connect();
      input = socket.getInputStream();
      output = socket.getOutputStream();

      DaemonCommunicator communicator = new DaemonCommunicator(input, output);
      communicator.send(request);

      Message responce = communicator.receive();
      if (responce == null)
        throw new ArchiveDaemonException("invalid_response");

      switch (responce.getType()) {
        case Message.SMSC_BYTE_EMPTY_TYPE:
          break;
        case Message.SMSC_BYTE_TOTAL_TYPE:
          smsCount = (int) ((TotalMessage) responce).getCount();
          break;
        case Message.SMSC_BYTE_ERROR_TYPE:
          logger.error("ArchiveDaemon communication error: " + ((ErrorMessage) responce).getError());
          throw new ArchiveDaemonException("error_returned");
        default:
          logger.error("Unknown response type: " + responce.getType());
          throw new ArchiveDaemonException("invalid_response");
      }
    } catch (IOException exc) {
      throw new ArchiveDaemonException("communication_error", exc);
    } finally {
      close(input, output, socket);
    }
    return smsCount;
  }

  private static void close(InputStream input, OutputStream output, Socket socket) {
    if (input != null) {
      try {
        input.close();
      } catch (IOException exc) {}
    }
    if (output != null) {
      try {
        output.close();
      } catch (IOException exc) {}
    }
    if (socket != null) {
      try {
        socket.close();
      } catch (IOException exc) {}
    }
  }
}
