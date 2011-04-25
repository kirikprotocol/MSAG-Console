package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.util.*;

/**
 * Управление рассылкой и статистикой сообщений
 *
 * @author Aleksandr Khalitov
 */
public class DeliveryManager implements UnmodifiableDeliveryManager{

  private static final Logger logger = Logger.getLogger(DeliveryManager.class);

  private final Map<User, DcpConnection> pool = new HashMap<User, DcpConnection>();

  private String host;

  private int port;


  public DeliveryManager(String host, int port) {
    this.host = host;
    this.port = port;
  }

  protected DeliveryManager() {
  }

  protected DcpConnection createConnection(String host, int port, String login, String password) throws AdminException {
    return new DcpConnection(host, port, login, password);
  }

  private synchronized DcpConnection getDcpConnection(String login, String password) throws AdminException {
    User u = new User(login, password);
    DcpConnection connection = pool.get(u);
    if (connection == null) {
      connection = createConnection(host, port, login, password);
      pool.put(u, connection);
    }
    return connection;
  }

  /**
   * Добавляет сообщения в рассылку с индивидуальными сообщениями
   * @param login логин
   * @param password пароль
   * @param msDataSource список сообщений, которые надо добавить
   * @param deliveryId идентификатор рассылки
   * @throws AdminException если произошла ошибка
   */
  public void addIndividualMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId) throws AdminException {
    addMessages(login, password, msDataSource, deliveryId, Delivery.Type.IndividualTexts);
  }

  /**
   * Добавляет реципиентов в рассылку с одним текстом
   * @param login логин
   * @param password пароль
   * @param msDataSource список адресов
   * @param deliveryId идентификатор рассылки
   * @throws AdminException если произошла ошибка
   */
  public void addSingleTextMessages(String login, String password, final DataSource<Address> msDataSource, int deliveryId) throws AdminException {
    addMessages(login, password, new Address2MessageDataSourceAdapter(msDataSource), deliveryId, Delivery.Type.SingleText);
  }

  private void addMessages(String login, String password, DataSource<Message> msDataSource, int deliveryId, Delivery.Type expectedDeliveryType) throws AdminException {
    Delivery d = getDelivery(login, password, deliveryId);
    if (d == null) {
      throw new DeliveryException("delivery_not_found");
    }
    if (d.getType() != expectedDeliveryType) {
      throw new DeliveryException("illegal_delivery_type");
    }
    addMessages(msDataSource, getDcpConnection(login, password), deliveryId);
  }

  private void addMessages(DataSource<Message> msDataSource, DcpConnection conn, int deliveryId) throws AdminException {
    Message m;
    int count = 0;
    List<Message> messages = new ArrayList<Message>(1000);
    while ((m = msDataSource.next()) != null) {
      messages.add(m);
      count++;
      if (count == 1000) {
        Collections.shuffle(messages);
        long[] ids = conn.addDeliveryMessages(deliveryId, messages);
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
      long[] ids = conn.addDeliveryMessages(deliveryId, messages);
      int i = 0;
      for (Message _m : messages) {
        _m.setId(ids[i]);
        i++;
      }
    }
  }


  public static void validateDeliveryWithIndividualTexts(DeliveryPrototype delivery) throws AdminException {
    Delivery d = new Delivery();
    d.setType(Delivery.Type.IndividualTexts);
    d.copyFrom(delivery);
    d.validate();
  }

  public static void validateDeliveryWithSingleText(DeliveryPrototype delivery) throws AdminException {
    Delivery d = new Delivery();
    d.setType(Delivery.Type.SingleText);
    d.copyFrom(delivery);
    d.validate();
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

  private int createDelivery(final String login, final String password, final Delivery delivery, final DataSource<Message> msDataSource) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Create delivery: " + delivery.getName());
    }

    if (delivery.getType() == Delivery.Type.SingleText)
      delivery.setProperty("singleText", "true");

    if (delivery.getStartDate().getTime() <= System.currentTimeMillis() + 10)
      delivery.setStartDate(new Date(System.currentTimeMillis() + 60000));

    delivery.validate();
    final DcpConnection conn = getDcpConnection(login, password);
    final int id = conn.createDelivery(delivery);

//    DeliveryState state = new DeliveryState();
//    state.setStatus(DeliveryStatus.Paused);
//    state.setDate(new Date());
//    conn.changeDeliveryState(id, state);

    delivery.setId(id);
    if (msDataSource != null) {
      try {
        if (delivery.getType() == Delivery.Type.SingleText)
          conn.modifyDeliveryGlossary(id, delivery.getSingleText());

        addMessages(msDataSource, conn, id);
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
      logger.debug("Delivery created: " + id);
    }

    return id;
  }

  /**
   * Создаёт рассылку с индивидуальными текстами для каждого реципиента.
   *
   * @param login        логин
   * @param password     пароль
   * @param delivery     рассылка
   * @param msDataSource сообщения
   * @return созданную рассылку
   * @throws mobi.eyeline.informer.admin.AdminException
   *          ошибка выполнения команды
   */
  public Delivery createDeliveryWithIndividualTexts(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    Delivery d = new Delivery();
    d.setType(Delivery.Type.IndividualTexts);
    d.setLoaded(true);
    d.copyFrom(delivery);
    createDelivery(login, password, d, msDataSource);
    return d;
  }

  /**
   * Создает рассылку с одним текстом для всех реципиентов.
   *
   * @param login      логин
   * @param password   пароль
   * @param delivery   рассылка
   * @param dataSource адресаты рассылки (или null)
   * @return созданную рассылку
   * @throws AdminException ошибка выполнения команды
   */
  public Delivery createDeliveryWithSingleText(String login, String password, DeliveryPrototype delivery, DataSource<Address> dataSource) throws AdminException {
    Delivery d = new Delivery();
    d.setType(Delivery.Type.SingleText);
    d.setLoaded(true);
    d.copyFrom(delivery);
    createDelivery(login, password, d, new Address2MessageDataSourceAdapter(dataSource));
    return d;
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

    delivery.validate();
    DcpConnection conn = getDcpConnection(login, password);
    conn.modifyDelivery(delivery);

    if (delivery.getType() == Delivery.Type.SingleText)
      conn.modifyDeliveryGlossary(delivery.getId(), delivery.getSingleText());
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
    DcpConnection conn = getDcpConnection(login, password);
    conn.dropDelivery(deliveryId);
  }

  /**
   * Архивация рассылки
   *
   * @param login      логин
   * @param password   пароль
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void archivateDelivery(String login, String password, int deliveryId) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Archivate delivery with id: " + deliveryId);
    }
    DcpConnection conn = getDcpConnection(login, password);
    conn.archivateDelivery(deliveryId);
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
    DcpConnection conn = getDcpConnection(login, password);
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
    DcpConnection conn = getDcpConnection(login, password);
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
    DcpConnection conn = getDcpConnection(login, password);
    Delivery d = conn.getDelivery(deliveryId);
    if (d == null)
      return null;
    d.setLoaded(true);

    d.setDeliveryManager(this);
    d.setLogin(login);
    d.setPassword(password);

    if (d.getProperty("singleText") != null) {
      d.setType(Delivery.Type.SingleText);
      d.setSingleText(conn.getDeliveryGlossary(deliveryId)[0]);
    } else
      d.setType(Delivery.Type.IndividualTexts);

    return d;
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
    Delivery d = getDelivery(login, password, deliveryId);
    if (d != null) {
      Date planDate = d.getStartDate();
      if (planDate.getTime() < System.currentTimeMillis())
        planDate = new Date();
      DeliveryState state = new DeliveryState();
      state.setStatus(DeliveryStatus.Planned);
      state.setDate(planDate);
      changeDeliveryState(login, password, deliveryId, state);
    } else
      throw new DeliveryException("delivery_not_found");
  }

  private void changeDeliveryState(String login, String password, int deliveryId, DeliveryState state) throws AdminException {
    DcpConnection conn = getDcpConnection(login, password);
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
    DcpConnection conn = getDcpConnection(login, password);
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
  public void getDeliveries(final String login, final String password, DeliveryFilter deliveryFilter, int _pieceSize, Visitor<Delivery> visitor) throws AdminException {
    if (deliveryFilter == null) {
      throw new DeliveryException("resultFields");
    }
    DcpConnection conn = getDcpConnection(login, password);
    int _reqId = conn.getDeliveries(deliveryFilter);
    new VisitorHelper<Delivery>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<Delivery> result) throws AdminException {
        boolean res = connection.getNextDeliveries(reqId, pieceSize, result);
        for (Delivery d : result) {
          d.setDeliveryManager(DeliveryManager.this);
          d.setLogin(login);
          d.setPassword(password);
          d.setLoaded(false);

          if (d.getProperty("singleText") != null) {
            d.setType(Delivery.Type.SingleText);
          } else
            d.setType(Delivery.Type.IndividualTexts);
        }

        return res;
      }
    }.visit(visitor);
  }

  private static MessageFilter prepareFilter(MessageFilter original) {
    MessageFilter result = new MessageFilter(original);

    if (result.getErrorCodes() != null && result.getErrorCodes().length > 0) {

      if (result.getStates() == null || result.getStates().length == 0) {
        result.setStates(MessageState.Delivered, MessageState.Expired, MessageState.Failed);

      } else {
        ArrayList<MessageState> states = new ArrayList<MessageState>(Arrays.asList(result.getStates()));
        states.remove(MessageState.New);
        states.remove(MessageState.Process);
        result.setStates(states.toArray(new MessageState[states.size()]));
      }

    }
    return result;
  }

  private static GetMessagesStrategy selectGetMessagesStrategy(MessageFilter filter) {
    if (filter.getStates() == null)
      return new GetNonFinalMessagesStrategy();
    for (MessageState s : filter.getStates())
      if (s == MessageState.New || s == MessageState.Process)
        return new GetNonFinalMessagesStrategy();

    return new GetFinalizedMessagesStrategy();
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
  public void getMessages(String login, String password, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {
    if (filter == null) {
      throw new DeliveryException("resultFields");
    }
    if (filter.getStartDate() == null || filter.getEndDate() == null) {
      throw new DeliveryException("date_start_end_empty");
    }

    MessageFilter preparedFilter = prepareFilter(filter);

    GetMessagesStrategy strategy = selectGetMessagesStrategy(preparedFilter);
    DcpConnection conn = getDcpConnection(login, password);
    strategy.getMessages(conn, preparedFilter, _pieceSize, visitor);
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
    GetMessagesStrategy strategy = selectGetMessagesStrategy(messageFilter);
    DcpConnection conn = getDcpConnection(login, password);
    return strategy.countMessages(conn, messageFilter);
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
    DcpConnection conn = getDcpConnection(login, password);
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

  /**
   * Адаптер для преобразования DataSource&lt;Address&gt; в DataSource%lt;Message&gt;
   */
  private static class Address2MessageDataSourceAdapter implements DataSource<Message> {
    private final DataSource<Address> addrDs;

    private Address2MessageDataSourceAdapter(DataSource<Address> addrDs) {
      this.addrDs = addrDs;
    }

    public Message next() throws AdminException {
      Address addr = addrDs.next();
      if (addr == null)
        return null;
      Message m = new Message();
      m.setAbonent(addr);
      m.setGlossaryIndex(0);
      return m;
    }
  }

  /**
   * Составной ключ для пользователя
   */
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
