package ru.novosoft.smsc.admin.archive_daemon;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.archive_daemon.messages.*;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.ServiceManager;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

@SuppressWarnings({"EmptyCatchBlock"})

/**
 * API для простмотра статистики смс
 * @author Aleksandr Khalitov
 */

public class ArchiveDemon {

  private final static int MAX_SMS_FETCH_SIZE = 5000;

  private final ArchiveConfig context;

  private static ArchiveDemon instance;

  private static final Logger logger = Logger.getLogger(ArchiveDemon.class);

  private static final String SERVICE_NAME = "ArchiveDaemon";

  private static final Lock initLock = new ReentrantLock();


  /**
   * Возвращает инстанс класса
   *
   * @return инстанс класса
   * @throws AdminException ошибка при инициализации
   */
  public static ArchiveDemon getInstance() throws AdminException {
    if (instance == null) {
      try {
        initLock.lock();
        if (instance == null) {
          ServiceInfo si = ServiceManager.getInstance().getService(SERVICE_NAME);
          String host;
          if (si == null || (host = si.getOnlineHost()) != null) {
            throw new AdminException("Archive demon is offline");
          }
          instance = new ArchiveDemon(host, new File(si.getBaseDir(), "conf" + File.separatorChar + "daemon.xml"));
        }
      } finally {
        initLock.unlock();
      }
    }
    return instance;
  }

  /*
  todo ArchiveDaemon во время работы может быть переключен с ноды на ноду. Поэтому информация о хосте должна
  todo грузиться в рантайме. Предлагаю делать так: при инициализации запомнить хост и пытаться отправлять запросы
  todo на этот хост. Если коннект с демоном установить не удалось, то хост запрашивается повторно из ServiceManager-а.
   */
  private ArchiveDemon(String host, File config) throws AdminException {
    context = new ArchiveConfig(host, config);
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

      socket = new Socket(/*"sunfire"*/context.getHost(), context.getPort());
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

      socket = new Socket(/*"sunfire"*/context.getHost(), context.getPort());
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

  /**
   * Возвращает настройки ArchiveDemon
   *
   * @return настройки ArchiveDemon
   */
  public ArchiveConfig getContext() {
    return context;
  }
}
