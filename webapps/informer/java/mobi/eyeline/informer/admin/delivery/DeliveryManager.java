package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.*;

/**
 * Управление рассылкой и статистикой сообщений
 *
 * @author Aleksandr Khalitov
 */
public class DeliveryManager {

  private static final Logger logger = Logger.getLogger(DeliveryManager.class);

  private final Map<User, DcpConnection> pool = new HashMap<User, DcpConnection>();

  private String host;

  private int port;

  private final DeliveryStatProvider statsProvider;


  public DeliveryManager(String host, int port, File statsDirectory, FileSystem fileSys) {
    this.host = host;
    this.port = port;
    this.statsProvider = new DeliveryStatProvider(statsDirectory, fileSys);
  }

  protected DeliveryManager(DeliveryStatProvider statsProvider) {
    this.statsProvider = statsProvider;
  }

  protected DcpConnection createConnection(String host, int port, String login, String password) throws AdminException {
    return new DcpConnection(host, port, login, password);
  }

  private synchronized DcpConnection getDeliveryConnection(String login, String password) throws AdminException {
    User u = new User(login, password);
    DcpConnection connection = pool.get(u);
    if (connection == null) {
      connection = createConnection(host, port, login, password);
      pool.put(u, connection);
    }
//    if (!connection.isConnected()) {
//      throw new DeliveryException("connection_lost");
//    }
    return connection;
  }

  public void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {
    DcpConnection conn = getDeliveryConnection(login, password);
    Delivery d = conn.getDelivery(deliveryId);
    if (d == null) {
      throw new DeliveryException("delivery_not_found");
    }
    if (d.getType() == Delivery.Type.SingleText) {
      throw new DeliveryException("illegal_delivery_type");
    }
    addMessages(msDataSource, conn, d);
  }

  public List<Long> addSingleTextMessages(String login, String password, DataSource<Address> msDataSource, int deliveryId) throws AdminException {
    DcpConnection conn = getDeliveryConnection(login, password);
    Delivery d = conn.getDelivery(deliveryId);
    if (d == null) {
      throw new DeliveryException("delivery_not_found");
    }
    if (d.getType() == Delivery.Type.Common) {
      throw new DeliveryException("illegal_delivery_type");
    }
    return addSingleTextMessages(msDataSource, conn, d);
  }

  private void addMessages(DataSource<Message> msDataSource, DcpConnection conn, Delivery delivery) throws AdminException {
    Message m;
    int count = 0;
    List<Message> messages = new ArrayList<Message>(1000);
    while ((m = msDataSource.next()) != null) {
      messages.add(m);
      count++;
      if (count == 1000) {
        Collections.shuffle(messages);
        long[] ids = conn.addDeliveryMessages(delivery.getId(), messages);
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
      Collections.shuffle(messages);
      long[] ids = conn.addDeliveryMessages(delivery.getId(), messages);
      int i = 0;
      for (Message _m : messages) {
        _m.setId(ids[i]);
        i++;
      }
    }
  }

  private List<Long> addSingleTextMessages(DataSource<Address> dataSource, DcpConnection conn, Delivery delivery) throws AdminException {
    Address a;
    int count = 0;
    List<Address> addresses = new ArrayList<Address>(1000);
    List<Long> results = new LinkedList<Long>();
    while ((a = dataSource.next()) != null) {
      addresses.add(a);
      count++;
      if (count == 1000) {
        Collections.shuffle(addresses);
        long[] ids = conn.addDeliveryAddresses(delivery.getId(), addresses);
        for (long id : ids) {
          results.add(id);
        }
        addresses.clear();
        count = 0;
      }
    }
    if (!addresses.isEmpty()) {
      Collections.shuffle(addresses);
      long[] ids = conn.addDeliveryAddresses(delivery.getId(), addresses);
      for (long id : ids) {
        results.add(id);
      }
    }
    return results;
  }

  private static void validateDelivery(final Delivery delivery) throws AdminException {
    if (delivery.isReplaceMessage() && (delivery.getSvcType() == null || delivery.getSvcType().length() == 0)) {
      throw new DeliveryException("replace_illegal");
    }
    if ((delivery.getActivePeriodStart() == null || delivery.getActivePeriodEnd() == null)) {
      throw new DeliveryException("active_period_illegal");
    }
    if (delivery.getSourceAddress() == null) {
      throw new DeliveryException("source_address_empty");
    }
    if (delivery.getStartDate() == null) {
      throw new DeliveryException("start_date_empty");
    }
    if (delivery.getOwner() == null) {
      throw new DeliveryException("owner_empty");
    }
    if (delivery.getName() == null) {
      throw new DeliveryException("name_empty");
    }
  }

  /**
   * Создание рассылки
   *
   * @param login        логин
   * @param password     пароль
   * @param delivery     рассылка
   * @param msDataSource сообщения
   * @throws mobi.eyeline.informer.admin.AdminException
   *          ошибка выполнения команды
   */
  public void createDelivery(final String login, final String password, final Delivery delivery, final DataSource<Message> msDataSource) throws AdminException {
    if (delivery.getType() == Delivery.Type.SingleText) {
      throw new DeliveryException("illegal_delivery_type");
    }
    if (logger.isDebugEnabled()) {
      logger.debug("Create delivery: " + delivery.getName());
    }
    validateDelivery(delivery);
    final DcpConnection conn = getDeliveryConnection(login, password);
    final int id = conn.createDelivery(delivery);
    delivery.setId(id);
    if (msDataSource != null) {
      try {
        addMessages(msDataSource, conn, delivery);
      } catch (AdminException e) {
        logger.error("Delivery creation failed.", e);
        silentDropDelivery(conn, id);
        throw e;

      } catch (Exception e) {
        logger.error("Delivery creation failed.", e);
        silentDropDelivery(conn, id);
        throw new DeliveryException("internal_error");
      }
    }
    if (logger.isDebugEnabled()) {
      logger.debug("Delivery is proccessed: " + id);
    }
  }

  private void silentDropDelivery(DcpConnection conn, int id) {
    try {
      logger.warn("Try drop delivery: " + id);
      conn.dropDelivery(id);
      logger.warn("Delivery is removed: " + id);
    } catch (Exception ex) {
      logger.error(ex, ex);
    }
  }

  /**
   * Создание рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param delivery   рассылка
   * @param dataSource адресаты рассылки (или null)
   * @return идентификаторы сообщений (или null, если спсико адресатов пуст)
   * @throws mobi.eyeline.informer.admin.AdminException
   *          ошибка выполнения команды
   */
  public List<Long> createSingleTextDelivery(final String login, final String password, final Delivery delivery, final DataSource<Address> dataSource) throws AdminException {
    if (delivery.getType() == Delivery.Type.Common) {
      throw new DeliveryException("illegal_delivery_type");
    }
    if (logger.isDebugEnabled()) {
      logger.debug("Create delivery: " + delivery.getName());
    }
    validateDelivery(delivery);
    final DcpConnection conn = getDeliveryConnection(login, password);
    final int id = conn.createDelivery(delivery);
    delivery.setId(id);
    List<Long> res = null;
    if (dataSource != null) {
      try {
        res = addSingleTextMessages(dataSource, conn, delivery);
      } catch (AdminException e) {
        logger.error("Delivery creation failed.", e);
        silentDropDelivery(conn, id);
        throw e;

      } catch (Exception e) {
        logger.error("Delivery creation failed.", e);
        silentDropDelivery(conn, id);
        throw new DeliveryException("internal_error");
      }
    }
    if (logger.isDebugEnabled()) {
      logger.debug("Delivery is proccessed: " + id);
    }
    return res;
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
    validateDelivery(delivery);
    DcpConnection conn = getDeliveryConnection(login, password);
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
    DcpConnection conn = getDeliveryConnection(login, password);
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
    DcpConnection conn = getDeliveryConnection(login, password);
    return conn.countDeliveries(deliveryFilter);
  }

  /**
   * Удаляет сообщения из рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @param messageIds идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public void dropMessages(String login, String password, int deliveryId, Collection<Long> messageIds) throws AdminException {
    DcpConnection conn = getDeliveryConnection(login, password);
    dropMessages(conn, deliveryId, messageIds);
  }

  private void dropMessages(DcpConnection conn, int deliveryId, Collection<Long> messageIds) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Drop Messages");
    }
    long[] ids = new long[messageIds.size()];
    int i = 0;
    for (Long l : messageIds) {
      ids[i] = l;
      i++;
    }
    conn.dropMessages(deliveryId, ids);
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
    DcpConnection conn = getDeliveryConnection(login, password);
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
    state.setDate(new Date());
    changeDeliveryState(login, password, deliveryId, state);
  }

  private void changeDeliveryState(String login, String password, int deliveryId, DeliveryState state) throws AdminException {
    DcpConnection conn = getDeliveryConnection(login, password);
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
    DcpConnection conn = getDeliveryConnection(login, password);
    return conn.getDeliveryState(deliveryId);
  }

  /**
   * Возвращает рассылки, удовлетворяющие фильтру
   *
   * @param login          логин
   * @param password       пароль
   * @param deliveryFilter фильтр
   * @param _pieceSize     сколько рассылок извлекать за одну транзакцию
   * @param visitor        визитер извлечения рассылок
   * @throws AdminException ошибка выполнения команды
   */
  public void getDeliveries(String login, String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<DeliveryInfo> visitor) throws AdminException {
    if (deliveryFilter == null || deliveryFilter.getResultFields() == null || deliveryFilter.getResultFields().length == 0) {
      throw new DeliveryException("resultFields");
    }
    DcpConnection conn = getDeliveryConnection(login, password);
    int _reqId = conn.getDeliveries(deliveryFilter);
    new DeliveryDataSource<DeliveryInfo>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<DeliveryInfo> result) throws AdminException {
        return connection.getNextDeliviries(reqId, pieceSize, result);
      }
    }.visit(visitor);
  }

  /**
   * Возвращает информацию о сообщениях рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param filter     фильтр
   * @param _pieceSize сколько рассылок извлекать за одну транзакцию
   * @param visitor    визитер извлечения сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public void getMessages(String login, String password, MessageFilter filter, int _pieceSize, Visitor<MessageInfo> visitor) throws AdminException {
    if (filter == null || filter.getFields() == null || filter.getFields().length == 0) {
      throw new DeliveryException("resultFields");
    }
    if (filter.getStartDate() == null || filter.getEndDate() == null) {
      throw new DeliveryException("date_start_end_empty");
    }
    DcpConnection conn = getDeliveryConnection(login, password);
    int _reqId = conn.getMessages(filter);
    final Delivery d = conn.getDelivery(filter.getDeliveryId());
    new DeliveryDataSource<MessageInfo>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<MessageInfo> result) throws AdminException {
        return connection.getNextMessages(reqId, pieceSize, d, result);
      }
    }.visit(visitor);
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
    DcpConnection conn = getDeliveryConnection(login, password);
    return conn.countMessages(messageFilter);
  }

  /**
   * Возвращает историю изменения статусов рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @return история изменения статусов рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryStatusHistory getDeliveryStatusHistory(String login, String password, int deliveryId) throws AdminException {
    DcpConnection conn = getDeliveryConnection(login, password);
    return conn.getDeliveryHistory(deliveryId);
  }

  /**
   * Поочередно передает в visitor все записи статистики, удовлетворяющие условиям, накладываемыми в filter.
   * Процесс продолжается до тех пор, пока метод visit в visitor возвращает true, либо записи не закончатся.
   * Если filter == null, то провайдер перебирает все записи.
   *
   * @param filter  фильтр, описывающий ограничения на записи
   * @param visitor визитор, обрабатывающий найденные записи
   * @throws AdminException если произошла ошибка при обращении к стораджу статистики
   */
  public void statistics(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException {
    statsProvider.accept(filter, visitor);
  }


  /**
   * Устанавливает у рассылки флаг в поле userData попадает она под запрет или нет
   *
   * @param login       логин
   * @param password    пароль
   * @param deliveryId  идентификатор рассылки
   * @param restriction флаг
   * @return рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public Delivery setDeliveryRestriction(String login, String password, int deliveryId, boolean restriction) throws AdminException {
    DcpConnection conn = getDeliveryConnection(login, password);
    Delivery d = conn.getDelivery(deliveryId);
    if (d == null) {
      throw new DeliveryException("delivery_not_found");
    }
    d.setProperty(UserDataConsts.RESTRICTION, Boolean.toString(restriction));
    conn.modifyDelivery(d);
    return d;
  }

  /**
   * Завершение работы менеджера
   */
  public void shutdown() {
    for (DcpConnection conn : pool.values()) {
      try {
        conn.close();
      } catch (Exception ignored) {
      }
    }
  }

  private static class User {
    private final String login;
    private final String password;

    private User(String login, String password) {
      this.login = login;
      this.password = password;
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) return true;
      if (o == null || getClass() != o.getClass()) return false;
      User user = (User) o;
      return !(login != null ? !login.equals(user.login) : user.login != null) &&
          !(password != null ? !password.equals(user.password) : user.password != null);
    }

    @Override
    public int hashCode() {
      int result = login != null ? login.hashCode() : 0;
      result = 31 * result + (password != null ? password.hashCode() : 0);
      return result;
    }
  }

}
