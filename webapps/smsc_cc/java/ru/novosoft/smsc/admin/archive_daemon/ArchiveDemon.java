package ru.novosoft.smsc.admin.archive_daemon;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.messages.*;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

@SuppressWarnings({"EmptyCatchBlock"})

/**
 * API для простмотра статистики смс
 * @author Aleksandr Khalitov
 */

public class ArchiveDemon {

  private final static int MAX_SMS_FETCH_SIZE = 5000;

  private static final Logger logger = Logger.getLogger(ArchiveDemon.class);

  public static final String SERVICE_ID = "ArchiveDaemon";

  private final ServiceManager serviceManager;
  private final FileSystem fileSystem;

  public ArchiveDemon(ServiceManager serviceManager, FileSystem fileSystem) throws AdminException {
    this.serviceManager = serviceManager;
    this.fileSystem = fileSystem;
  }

  private Socket connect() throws AdminException, IOException {
    ServiceInfo info = serviceManager.getService(SERVICE_ID);
    if (info == null)
      throw new AdminException("Service '" + SERVICE_ID + "' not found.");
    if (info.getOnlineHost() == null)
      throw new AdminException("Service '" + SERVICE_ID + "' is offline.");

    ArchiveDaemonConfig config = new ArchiveDaemonConfig(info.getBaseDir(), fileSystem);
    String host = info.getOnlineHost();
    int port = config.getViewPort();

    return new Socket(host, port);
  }

  /**
   * Возвращает статистику смс, удовлетворяющую запросу
   *
   * @param query запрос
   * @return статистика смс
   * @throws AdminException ошибка извлечения статистики
   */
  public SmsSet getSmsSet(SmsQuery query) throws AdminException {
    Socket socket = null;
    InputStream input = null;
    OutputStream output = null;

    SmsSet set = new SmsSet();
    set.setHasMore(false);
    int rowsMaximum = query.getRowsMaximum();
    if (rowsMaximum == 0) return set;

    QueryMessage request = new QueryMessage(query);

    try {

      socket = connect();
      input = socket.getInputStream();
      output = socket.getOutputStream();

      DaemonCommunicator communicator = new DaemonCommunicator(input, output);
      communicator.send(request);
      int toReceive = rowsMaximum < MAX_SMS_FETCH_SIZE ? rowsMaximum : MAX_SMS_FETCH_SIZE;
      communicator.send(new RsNxtMessage(toReceive));

      Message responce;
      boolean allSelected = false;
      do {
        responce = communicator.receive();
        if (responce == null) throw new AdminException("Message from archive daemon is NULL");

        switch (responce.getType()) {
          case Message.SMSC_BYTE_EMPTY_TYPE:
            allSelected = true;
            break;
          case Message.SMSC_BYTE_RSSMS_TYPE:
            set.addRow(((RsSmsMessage) responce).getSms());
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
            throw new AdminException("Archive daemon communication error: " +
                ((ErrorMessage) responce).getError());
          default:
            throw new AdminException("Unsupported message received from archive daemon, type: " +
                responce.getType());
        }
      } while (!allSelected);

    }
    catch (Exception exc) {
      logger.error(exc, exc);
      throw new AdminException(exc.getMessage());
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
  public int getSmsCount(SmsQuery query) throws AdminException {
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
        throw new AdminException("Message from archive daemon is NULL");

      switch (responce.getType()) {
        case Message.SMSC_BYTE_EMPTY_TYPE:
          break;
        case Message.SMSC_BYTE_TOTAL_TYPE:
          smsCount = (int) ((TotalMessage) responce).getCount();
          break;
        case Message.SMSC_BYTE_ERROR_TYPE:
          throw new AdminException("Archive daemon communication error: " +
              ((ErrorMessage) responce).getError());
        default:
          throw new AdminException("Unsupported message received from archive daemon, type: " +
              responce.getType());
      }
    }
    catch (Exception exc) {
      logger.error(exc, exc);
      throw new AdminException(exc.getMessage());
    } finally {
      close(input, output, socket);
    }
    return smsCount;
  }

  private static void close(InputStream input, OutputStream output, Socket socket) {
    if (input != null) {
      try {
        input.close();
      }
      catch (IOException exc) {
      }
    }
    if (output != null) {
      try {
        output.close();
      }
      catch (IOException exc) {
      }
    }
    if (socket != null) {
      try {
        socket.close();
      }
      catch (IOException exc) {
      }
    }
  }
}
