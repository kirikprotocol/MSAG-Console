package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Logger;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Управление рассылкой и статистикой сообщений
 *
 * @author Aleksandr Khalitov
 */
public class DeliveryManager {

  private static final Logger logger = Logger.getLogger(DeliveryManager.class);

  private final DcpConnectionFactory connectionFactory;

  public DeliveryManager(String host, int port) {
    this.connectionFactory = new DcpConnectionFactory(host, port);
  }

  protected DeliveryManager(DcpConnectionFactory factory) {
    this.connectionFactory = factory;
  }

  private DcpConnection getDeliveryConnection(String login, String password) throws AdminException {
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    if (!conn.isConnected()) {
      throw new DeliveryException("connection_lost");
    }
    return conn;
  }


  private void addMessages(int id, MessageDataSource msDataSource, DcpConnection conn) throws AdminException {
    Message m;
    int count = 0;
    List<Message> messages = new ArrayList<Message>(1000);
    while ((m = msDataSource.next()) != null) {
      messages.add(m);
      count++;
      if (count == 1000) {
        long[] ids = conn.addDeliveryMessages(id, messages.toArray(new Message[messages.size()]));
        int i = 0;
        for (Message _m : messages) {
          _m.setId(ids[i]);
          i++;
        }
        messages.clear();
        count = 0;
      }
    }
    if (!messages.isEmpty()) {
      long[] ids = conn.addDeliveryMessages(id, messages.toArray(new Message[messages.size()]));
      int i = 0;
      for (Message _m : messages) {
        _m.setId(ids[i]);
        i++;
      }
    }
  }

  /**
   * Создание рассылки
   *
   * @param login        логин
   * @param password     пароль
   * @param delivery     рассылка
   * @param msDataSource сообщения
   * @param glossary     глоссарий
   * @throws mobi.eyeline.informer.admin.AdminException
   *          ошибка выполнения команды
   */
  public void createDelivery(final String login, final String password, final Delivery delivery, final MessageDataSource msDataSource, final String[] glossary) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Create delivery: " + delivery.getName());
    }
    if (delivery.isRetryOnFail() && (delivery.getRetryPolicy() == null || delivery.getRetryPolicy().length() == 0)) {
      throw new DeliveryException("retry_illegal");
    }
    if (delivery.isReplaceMessage() && (delivery.getSvcType() == null || delivery.getSvcType().length() == 0)) {
      throw new DeliveryException("replace_illegal");
    }
    if (delivery.getValidityDate() == null && delivery.getValidityPeriod() == null) {
      throw new DeliveryException("validation_illegal");
    }
    if ((delivery.getActivePeriodStart() == null && delivery.getActivePeriodEnd() != null) ||
        (delivery.getActivePeriodStart() != null && delivery.getActivePeriodEnd() == null)) {
      throw new DeliveryException("active_period_illegal");
    }
    final DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    final int id = conn.createDelivery(delivery);
    delivery.setId(id);
    if (glossary != null) {
      conn.modifyDeliveryGlossary(id, glossary);
    }
    try {
      addMessages(id, msDataSource, conn);
    } catch (Exception e) {
      logger.error(e, e);
      try {
        logger.warn("Try drop delivery: " + id);
        conn.dropDelivery(id);
        logger.warn("Delivery is removed: " + id);
      } catch (Exception ex) {
        logger.error(ex, ex);
      }
      if (logger.isDebugEnabled()) {
        logger.debug("Delivery's processing failed: " + id);
      }
      throw new DeliveryException("internal_error");
    }
    if (logger.isDebugEnabled()) {
      logger.debug("Delivery is proccessed: " + id);
    }
  }

  /**
   * Модификация рассылки
   *
   * @param login    логин
   * @param password пароль
   * @param delivery рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public void modifyDelivery(String login, String password, Delivery delivery) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Modify delivery: " + delivery.getName());
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    conn.modifyDelivery(delivery);
  }

  /**
   * Удаление рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void dropDelivery(String login, String password, int deliveryId) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Drop delivery with id: " + deliveryId);
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    conn.dropDelivery(deliveryId);
  }

  /**
   * Подсчёт кол-ва рассылок
   *
   * @param login          логин
   * @param password       пароль
   * @param deliveryFilter фильтр
   * @return кол-во рассылок
   * @throws AdminException ошибка выполнения команды
   */
  public int countDeliveries(String login, String password, DeliveryFilter deliveryFilter) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Count deliveries");
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    return conn.countDeliveries(deliveryFilter);
  }

  /**
   * Удаляет сообщения из рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param messageIds идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public void dropMessages(String login, String password, long[] messageIds) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Drop Messages");
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    conn.dropMessages(messageIds);
  }

  /**
   * Возвращает глоссарий по рассылке
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентикатор рассылки
   * @return глоссарий
   * @throws AdminException ошибка выполнения команды
   */
  public String[] getDeliveryGlossary(String login, String password, int deliveryId) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Get deliveries glossary for: " + deliveryId);
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    return conn.getDeliveryGlossary(deliveryId);
  }

  /**
   * Возвращает рассылку по идентификатору
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @return рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Get delivery: " + deliveryId);
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    return conn.getDelivery(deliveryId);
  }

  /**
   * Отмена рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void cancelDelivery(String login, String password, int deliveryId) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Cancel delivery: " + deliveryId);
    }
    DeliveryState state = new DeliveryState();
    state.setStatus(DeliveryStatus.Cancelled);
    changeDeliveryState(login, password, deliveryId, state);
  }

  /**
   * Приостановка рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void pauseDelivery(String login, String password, int deliveryId) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Pause delivery: " + deliveryId);
    }
    DeliveryState state = new DeliveryState();
    state.setStatus(DeliveryStatus.Paused);
    changeDeliveryState(login, password, deliveryId, state);
  }

  /**
   * Запуск рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void activateDelivery(String login, String password, int deliveryId) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Activate delivery: " + deliveryId);
    }
    DeliveryState state = new DeliveryState();
    state.setStatus(DeliveryStatus.Active);
    changeDeliveryState(login, password, deliveryId, state);
  }

  private void changeDeliveryState(String login, String password, int deliveryId, DeliveryState state) throws AdminException {
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    conn.changeDeliveryState(deliveryId, state);
  }

  /**
   * Возвращает статистику по рассылке
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @return статистика по рассылке
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryStatistics getDeliveryStats(String login, String password, int deliveryId) throws AdminException {
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    return conn.getDeliveryState(deliveryId);
  }

  /**
   * Возвращает рассылки, удовлетворяющие фильтру
   *
   * @param login          логин
   * @param password       пароль
   * @param deliveryFilter фильтр
   * @param _pieceSize     сколько рассылок извлекать за одну транзакцию
   * @return рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryDataSource<DeliveryInfo> getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize) throws AdminException {
    if (deliveryFilter == null || deliveryFilter.getResultFields() == null || deliveryFilter.getResultFields().length == 0) {
      throw new DeliveryException("resultFields");
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    int _reqId = conn.getDeliviries(deliveryFilter);
    return new DeliveryDataSource<DeliveryInfo>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<DeliveryInfo> result) throws AdminException {
        return connection.getNextDeliviries(reqId, pieceSize, result);
      }
    };
  }

  /**
   * Возвращает информацию о сообщениях рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param filter     фильтр
   * @param _pieceSize сколько рассылок извлекать за одну транзакцию
   * @return информация о сообщениях рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryDataSource<MessageInfo> getMessagesStates(String login, String password, MessageFilter filter, int _pieceSize) throws AdminException {
    if (filter == null || filter.getFields() == null || filter.getFields().length == 0) {
      throw new DeliveryException("resultFields");
    }
    if (filter.getStartDate() == null || filter.getEndDate() == null) {
      throw new DeliveryException("date_start_end_empty");
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    int _reqId = conn.getMessagesStates(filter);
    return new DeliveryDataSource<MessageInfo>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<MessageInfo> result) throws AdminException {
        return connection.getNextMessageStates(reqId, pieceSize, result);
      }
    };
  }

  /**
   * Подсчёт кол-ва сообщений
   *
   * @param login         логин
   * @param password      пароль
   * @param messageFilter фильтр
   * @return кол-во сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public int countMessages(String login, String password, MessageFilter messageFilter) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Count deliveries");
    }
    DcpConnection conn = connectionFactory.getDeliveryConnection(login, password);
    return conn.countMessages(messageFilter);
  }

  /**
   * Завершение работы менеджера
   */
  public void shutdown() {
    if (connectionFactory != null) {
      try {
        connectionFactory.shutdown();
      } catch (Exception ignored) {
      }
    }

  }

}
