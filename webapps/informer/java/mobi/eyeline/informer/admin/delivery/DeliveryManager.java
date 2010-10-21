package mobi.eyeline.informer.admin.delivery;

import com.eyeline.utils.ThreadFactoryWithCounter;
import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Logger;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryManager {

  private static final Logger logger = Logger.getLogger(DeliveryManager.class);

  private final DcpConnectionFactory connectionFactory;

  private Executor executor =  new ThreadPoolExecutor(2, Integer.MAX_VALUE, 300, TimeUnit.SECONDS,
      new LinkedBlockingQueue<Runnable>(), new ThreadFactoryWithCounter("DeliveryCreater"));

  public DeliveryManager(String host, int port) {
    this.connectionFactory = new DcpConnectionFactory(host, port);
  }

  protected DeliveryManager(DcpConnectionFactory factory) {
    this.connectionFactory = factory;
  }

  private DcpConnection getDeliveryConnection(String login, String password) throws AdminException{
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    if(!conn.isConnected()) {
      throw new DeliveryException("connection_lost");
    }
    return conn;
  }

  /**
   * Создание рассылки
   * @param login логин
   * @param password пароль
   * @param delivery рассылка
   * @param msProcessing сообщения
   * @throws mobi.eyeline.informer.admin.AdminException ошибка выполнения команды
   */
  public void createDelivery(String login, String password, Delivery delivery, MessageProcessing msProcessing) throws AdminException {
    createDelivery(login, password, delivery, msProcessing, null);
  }


  private void addMessages(int id, MessageProcessing msProcessing, DcpConnection conn) {
    try{
      Message m;
      int count = 0;
      List<Message> messages = new ArrayList<Message>(1000);
      while((m = msProcessing.getToProcess()) != null) {
        messages.add(m);
        count++;
        if(count == 1000) {
          conn.addDeliveryMessages(id, messages.toArray(new Message[messages.size()]));
          msProcessing.incrementProcessed(count);
          messages.clear();
          count = 0;
        }
      }
      if(!messages.isEmpty()) {
        conn.addDeliveryMessages(id, messages.toArray(new Message[messages.size()]));
        msProcessing.incrementProcessed(count);
      }
      DeliveryState state = new DeliveryState();
      state.setStatus(DeliveryStatus.ACTIVE);
      conn.changeDeliveryState(id, state);
      msProcessing.success();
    }catch (Exception e) {
      logger.error(e,e);
      try{
        logger.warn("Try drop delivery: "+id);
        conn.dropDelivery(id);
        logger.warn("Delivery is removed: "+id);
      }catch (Exception ex) {
        logger.error(ex,ex);
      }
      msProcessing.error(e);
    }
  }

  /**
   * Создание рассылки
   * @param login логин
   * @param password пароль
   * @param delivery рассылка
   * @param msProcessing сообщения
   * @param glossary глоссарий
   * @throws mobi.eyeline.informer.admin.AdminException ошибка выполнения команды
   */
  public void createDelivery(final String login, final String password, final Delivery delivery, final MessageProcessing msProcessing, final String[] glossary) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Create delivery: "+delivery.getName());
    }
    final DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    final int id = conn.createDelivery(delivery);
    delivery.setId(id);
    if(glossary != null) {
      conn.modifyDeliveryGlossary(id, glossary);
    }
    executor.execute(new Runnable() {
      public void run() {
        addMessages(id, msProcessing, conn);
      }
    });
  }

  /**
   * Модификация рассылки
   * @param login логин
   * @param password пароль
   * @param delivery рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Modify delivery: "+delivery.getName());
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    conn.modifyDelivery(delivery);
  }

  /**
   * Удаление рассылки
   * @param login логин
   * @param password пароль
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Drop delivery with id: "+deliveryId);
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    conn.dropDelivery(deliveryId);
  }

  /**
   * Подсчёт кол-ва рассылок
   * @param login логин
   * @param password пароль
   * @param deliveryFilter фильтр
   * @throws AdminException ошибка выполнения команды
   * @return кол-во рассылок
   */
  public int countDeliveries(String login, String password, DeliveryFilter deliveryFilter) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Count deliveries");
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    return conn.countDeliveries(deliveryFilter);
  }

  /**
   * Удаляет сообщения из рассылки
   * @param login логин
   * @param password пароль
   * @param messageIds идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public void dropMessages(String login, String password, long[] messageIds) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Drop Messages");
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    conn.dropMessages(messageIds);
  }

  /**
   * Возвращает глоссарий по рассылке
   * @param login логин
   * @param password пароль
   * @param deliveryId идентикатор рассылки
   * @return глоссарий
   * @throws AdminException ошибка выполнения команды
   */
  public String[] getDeliveryGlossary(String login, String password, int deliveryId) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Get deliveries glossary for: "+deliveryId);
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    return conn.getDeliveryGlossary(deliveryId);
  }

  /**
   * Возвращает рассылку по идентификатору
   * @param login логин
   * @param password пароль
   * @param deliveryId идентификатор рассылки
   * @return рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Get delivery: "+deliveryId);
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    return conn.getDelivery(deliveryId);
  }

  /**
   * Отмена рассылки
   * @param login логин
   * @param password пароль
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void cancelDelivery(String login, String password, int deliveryId) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Cancel delivery: "+deliveryId);
    }
    DeliveryState state = new DeliveryState();
    state.setStatus(DeliveryStatus.CANCELED);
    changeDeliveryState(login, password, deliveryId, state);
  }

  /**
   * Приостановка рассылки
   * @param login логин
   * @param password пароль
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void pauseDelivery(String login, String password, int deliveryId) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Pause delivery: "+deliveryId);
    }
    DeliveryState state = new DeliveryState();
    state.setStatus(DeliveryStatus.PAUSED);
    changeDeliveryState(login, password, deliveryId, state);
  }

  /**
   * Запуск рассылки
   * @param login логин
   * @param password пароль
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void activateDelivery(String login, String password, int deliveryId) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Activate delivery: "+deliveryId);
    }
    DeliveryState state = new DeliveryState();
    state.setStatus(DeliveryStatus.ACTIVE);
    changeDeliveryState(login, password, deliveryId, state);
  }

  private void changeDeliveryState (String login, String password, int deliveryId, DeliveryState state) throws AdminException {
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    conn.changeDeliveryState(deliveryId, state);
  }

  /**
   * Возвращает статистику по рассылке
   * @param login логин
   * @param password пароль
   * @param deliveryId идентификатор рассылки
   * @return статистика по рассылке
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryStatistics getDeliveryState(String login, String password, int deliveryId) throws AdminException {
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    return conn.getDeliveryState(deliveryId);
  }

  /**
   * Возвращает рассылки, удовлетворяющие фильтру
   * @param login логин
   * @param password пароль
   * @param deliveryFilter фильтр
   * @param _pieceSize сколько рассылок извлекать за одну транзакцию
   * @return рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryDataSource<DeliveryInfo> getDeliviries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize) throws AdminException {
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    int _reqId =  conn.getDeliviries(deliveryFilter);
    return new DeliveryDataSource<DeliveryInfo>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<DeliveryInfo> result) throws AdminException {
        return connection.getNextDeliviries(reqId, pieceSize, result);
      }
    };
  }

  /**
   * Возвращает информацию о сообщениях рассылки
   * @param login логин
   * @param password пароль
   * @param filter фильтр
   * @param _pieceSize сколько рассылок извлекать за одну транзакцию
   * @return информация о сообщениях рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryDataSource<MessageInfo> getMessagesStates(String login, String password, MessageFilter filter, int _pieceSize) throws AdminException {
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    int _reqId =  conn.getMessagesStates(filter);
    return new DeliveryDataSource<MessageInfo>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<MessageInfo> result) throws AdminException {
        return connection.getNextMessageStates(reqId, pieceSize, result);
      }
    };
  }

}
