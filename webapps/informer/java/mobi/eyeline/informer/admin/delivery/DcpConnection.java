package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.DcpClient;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields;
import mobi.eyeline.informer.admin.protogen.SyncProtogenConnection;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Коннект к DCP, упрвление рассылками
 *
 * @author Aleksandr Khalitov
 */
public class DcpConnection {

  private static final Logger logger = Logger.getLogger(DcpConnection.class);

  private DcpClient client;

  private Lock lock;

  public DcpConnection(String host, int port, final String login, String password) throws AdminException {
    this.client = new DcpClient(host, port, login, password);

    lock = new ReentrantLock() {
      @Override
      public void lock() {
        super.lock();
        if (logger.isDebugEnabled()) {
          logger.debug("Dcp Connection locked: login=" + login);
        }
      }

      @Override
      public void unlock() {
        super.unlock();
        if (logger.isDebugEnabled()) {
          logger.debug("Dcp Connection unlocked: login=" + login);
        }
      }
    };
  }

  protected DcpConnection() {
  }

  /**
   * Соединение установлено
   *
   * @return true - да, false - нет
   */
//  public boolean isConnected() {
//    return client.isConnected();
//  }

  /**
   * Закрывает соединение
   */
  public void close() {
    client.close();
  }

  /**
   * Создание рассылки
   *
   * @param delivery рассылка
   * @return идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public int createDelivery(Delivery delivery) throws AdminException {
    CreateDelivery req = new CreateDelivery();
    req.setInfo(DcpConverter.convert(delivery));
    CreateDeliveryResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    if (delivery.getSingleText() != null) {
      ModifyDeliveryGlossary reqG = new ModifyDeliveryGlossary();
      reqG.setDeliveryId(resp.getDeliveryId());
      DeliveryGlossary glossary = new DeliveryGlossary();
      glossary.setMessages(new String[]{delivery.getSingleText()});
      reqG.setGlossary(glossary);
      try {
        lock.lock();
        client.send(reqG);
      } finally {
        lock.unlock();
      }
    }
    return resp.getDeliveryId();
  }


  /**
   * Добавляет сообщения в рассылку
   *
   * @param deliveryId идентификатор рассылки
   * @param messages   сообщения
   * @return идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public long[] addDeliveryMessages(int deliveryId, List<Message> messages) throws AdminException {
    AddDeliveryMessages req = new AddDeliveryMessages();
    req.setDeliveryId(deliveryId);
    req.setMessages(DcpConverter.convert(messages));
    AddDeliveryMessagesResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return resp.getMessageIds();
  }


  /**
   * Добавляет сообщения в рассылку
   *
   * @param deliveryId идентификатор рассылки
   * @param addresses  адресаты
   * @return идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public long[] addDeliveryAddresses(int deliveryId, List<Address> addresses) throws AdminException {
    AddDeliveryMessages req = new AddDeliveryMessages();
    req.setDeliveryId(deliveryId);
    req.setMessages(DcpConverter.convert(addresses));
    AddDeliveryMessagesResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return resp.getMessageIds();
  }

  /**
   * Модификация рассылки
   *
   * @param delivery рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public void modifyDelivery(Delivery delivery) throws AdminException {
    ModifyDelivery req = new ModifyDelivery();
    req.setDeliveryId(delivery.getId());
    req.setInfo(DcpConverter.convert(delivery));
    try {
      lock.lock();
      client.send(req);
    } finally {
      lock.unlock();
    }
    if (delivery.getType() == Delivery.Type.SingleText) {
      ModifyDeliveryGlossary reqG = new ModifyDeliveryGlossary();
      reqG.setDeliveryId(delivery.getId());
      DeliveryGlossary glossary = new DeliveryGlossary();
      glossary.setMessages(new String[]{delivery.getSingleText()});
      reqG.setGlossary(glossary);
      try {
        lock.lock();
        client.send(reqG);
      } finally {
        lock.unlock();
      }
    }
  }

  /**
   * Удаление рассылки
   *
   * @param deliveryId идентификатор рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public void dropDelivery(int deliveryId) throws AdminException {
    DropDelivery req = new DropDelivery();
    req.setDeliveryId(deliveryId);
    try {
      lock.lock();
      client.send(req);
    } finally {
      lock.unlock();
    }
  }

  /**
   * Подсчёт кол-ва рассылок
   *
   * @param deliveryFilter фильтр
   * @return кол-во рассылок
   * @throws AdminException ошибка выполнения команды
   */
  public int countDeliveries(DeliveryFilter deliveryFilter) throws AdminException {
    CountDeliveries req = new CountDeliveries();
    DeliveriesFilter f = new DeliveriesFilter();
    if (deliveryFilter != null) {
      if (deliveryFilter.getEndDateFrom() != null) {
        f.setEndDateFrom(DcpConverter.convertDate(deliveryFilter.getEndDateFrom()));
      }
      if (deliveryFilter.getEndDateTo() != null) {
        f.setEndDateTo(DcpConverter.convertDate(deliveryFilter.getEndDateTo()));
      }
      if (deliveryFilter.getStartDateFrom() != null) {
        f.setStartDateFrom(DcpConverter.convertDate(deliveryFilter.getStartDateFrom()));
      }
      if (deliveryFilter.getStartDateTo() != null) {
        f.setStartDateTo(DcpConverter.convertDate(deliveryFilter.getStartDateTo()));
      }
      if (deliveryFilter.getNameFilter() != null && deliveryFilter.getNameFilter().length > 0) {
        f.setNameFilter(deliveryFilter.getNameFilter());
      }
      if (deliveryFilter.getUserIdFilter() != null && deliveryFilter.getUserIdFilter().length > 0) {
        f.setUserIdFilter(deliveryFilter.getUserIdFilter());
      }
      if (deliveryFilter.getStatusFilter() != null && deliveryFilter.getStatusFilter().length > 0) {
        f.setStatusFilter(DcpConverter.convert(deliveryFilter.getStatusFilter()));
      }
    }
    req.setFilter(f);
    CountDeliveriesResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return resp.getResult();
  }

  /**
   * Удаляет сообщения из рассылки
   *
   * @param deliveryId идентификатор рассылки
   * @param messageIds идентификаторы сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public void dropMessages(int deliveryId, long[] messageIds) throws AdminException {
    DropDeliveryMessages req = new DropDeliveryMessages();
    req.setMessageIds(messageIds);
    req.setDeliveryId(deliveryId);
    try {
      lock.lock();
      client.send(req);
    } finally {
      lock.unlock();
    }
  }

  /**
   * Возвращает рассылку по идентификатору
   *
   * @param deliveryId идентификатор рассылки
   * @return рассылка
   * @throws AdminException ошибка выполнения команды
   */
  public Delivery getDelivery(int deliveryId) throws AdminException {
    GetDeliveryInfo req = new GetDeliveryInfo();
    req.setDeliveryId(deliveryId);
    GetDeliveryInfoResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    if (!resp.hasInfo()) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo info = resp.getInfo();
    Map<String, String> userData = DcpConverter.convertUserData(info.getUserData());
    if (!userData.containsKey("singleText")) {
      return DcpConverter.convert(deliveryId, info, null);
    } else {
      GetDeliveryGlossary reqG = new GetDeliveryGlossary();
      GetDeliveryGlossaryResp respG;
      reqG.setDeliveryId(deliveryId);
      try {
        lock.lock();
        respG = client.send(reqG);
      } finally {
        lock.unlock();
      }
      return DcpConverter.convert(deliveryId, info, respG.getGlossary().getMessages());
    }
  }

  /**
   * Меняет состояние рассылки
   *
   * @param deliveryId идентификатор рассылки
   * @param state      новое состояние
   * @throws AdminException ошибка выполнения команды
   */
  public void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException {
    ChangeDeliveryState req = new ChangeDeliveryState();
    req.setDeliveryId(deliveryId);
    req.setState(DcpConverter.convert(state));
    try {
      lock.lock();
      client.send(req);
    } finally {
      lock.unlock();
    }
  }

  /**
   * Возвращает статистику по рассылке
   *
   * @param deliveryId идентификатор рассылки
   * @return статистика по рассылке
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
    GetDeliveryState req = new GetDeliveryState();
    req.setDeliveryId(deliveryId);
    GetDeliveryStateResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return DcpConverter.convert(resp.getStats(), resp.getState());
  }

  /**
   * Возвращает идентификатор запроса для извлечения рассылок, удовлетворяющие фильтру
   *
   * @param deliveryFilter фильтр
   * @return идентификатор запроса
   * @throws AdminException ошибка выполнения команды
   */
  public int getDeliveries(DeliveryFilter deliveryFilter) throws AdminException {
    GetDeliveriesList req = new GetDeliveriesList();
    DeliveriesFilter f = new DeliveriesFilter();
    if (deliveryFilter.getEndDateFrom() != null) {
      f.setEndDateFrom(DcpConverter.convertDate(deliveryFilter.getEndDateFrom()));
    }
    if (deliveryFilter.getStartDateFrom() != null) {
      f.setStartDateFrom(DcpConverter.convertDate(deliveryFilter.getStartDateFrom()));
    }
    if (deliveryFilter.getEndDateTo() != null) {
      f.setEndDateTo(DcpConverter.convertDate(deliveryFilter.getEndDateTo()));
    }
    if (deliveryFilter.getStartDateTo() != null) {
      f.setStartDateTo(DcpConverter.convertDate(deliveryFilter.getStartDateTo()));
    }
    if (deliveryFilter.getEndDateFrom() != null) {
      f.setEndDateFrom(DcpConverter.convertDate(deliveryFilter.getEndDateFrom()));
    }
    if (deliveryFilter.getEndDateTo() != null) {
      f.setEndDateTo(DcpConverter.convertDate(deliveryFilter.getEndDateTo()));
    }
    if (deliveryFilter.getStartDateFrom() != null) {
      f.setStartDateFrom(DcpConverter.convertDate(deliveryFilter.getStartDateFrom()));
    }
    if (deliveryFilter.getStartDateTo() != null) {
      f.setStartDateTo(DcpConverter.convertDate(deliveryFilter.getStartDateTo()));
    }
    if (deliveryFilter.getNameFilter() != null && deliveryFilter.getNameFilter().length > 0) {
      f.setNameFilter(deliveryFilter.getNameFilter());
    }
    if (deliveryFilter.getUserIdFilter() != null && deliveryFilter.getUserIdFilter().length > 0) {
      f.setUserIdFilter(deliveryFilter.getUserIdFilter());
    }
    if (deliveryFilter.getStatusFilter() != null && deliveryFilter.getStatusFilter().length > 0) {
      f.setStatusFilter(DcpConverter.convert(deliveryFilter.getStatusFilter()));
    }
    DeliveryFields[] fieldses = DcpConverter.convert(deliveryFilter.getResultFields());
    DeliveryFields[] fs = new DeliveryFields[fieldses.length + 1];
    fs[0] = DeliveryFields.UserData;
    System.arraycopy(fieldses, 0, fs, 1, fieldses.length);
    req.setResultFields(fs);
    req.setFilter(f);

    GetDeliveriesListResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return resp.getReqId();
  }

  /**
   * Возвращает следующую часть рассылок по идентификатору запроса
   *
   * @param reqId      идентификатор запроса
   * @param pieceSize  максимальное кол-во извлекаемых рассылок
   * @param deliveries куда следуют сложить рассылки
   * @return есть ли ещё рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public boolean getNextDeliviries(int reqId, int pieceSize, Collection<DeliveryInfo> deliveries) throws AdminException {
    GetDeliveriesListNext req = new GetDeliveriesListNext();
    req.setReqId(reqId);
    req.setCount(pieceSize);
    GetDeliveriesListNextResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    if (resp.getInfo() != null) {
      for (DeliveryListInfo di : resp.getInfo()) {
        deliveries.add(DcpConverter.convert(di));
      }
    }
    return resp.getMoreDeliveries();
  }

  /**
   * Возвращает идентифкатор запроса для извлечения информации о сообщениях рассылки
   *
   * @param filter фильтр
   * @return идентифкатор запроса
   * @throws AdminException ошибка выполнения команды
   */
  public int getMessages(MessageFilter filter) throws AdminException {
    RequestMessagesState req = new RequestMessagesState();
    if (filter != null) {
      if (filter.getDeliveryId() != null) {
        req.setDeliveryId(filter.getDeliveryId());
      }
      req.setEndDate(DcpConverter.convertDate(filter.getEndDate()));
      req.setStartDate(DcpConverter.convertDate(filter.getStartDate()));

      ReqField[] fieldses = DcpConverter.convert(filter.getFields());
      ReqField[] fs = new ReqField[fieldses.length + 1];
      fs[0] = ReqField.UserData;
      System.arraycopy(fieldses, 0, fs, 1, fieldses.length);
      req.setFields(fs);

      if (filter.getMsisdnFilter() != null && filter.getMsisdnFilter().length > 0) {
        req.setMsisdnFilter(filter.getMsisdnFilter());
      }
      if (filter.getStates() != null && filter.getStates().length > 0) {
        req.setStates(DcpConverter.convert(filter.getStates()));
      }
      if (filter.getErrorCodes() != null && filter.getErrorCodes().length > 0) {
        int[] codes = new int[filter.getErrorCodes().length];
        System.arraycopy(filter.getErrorCodes(), 0, codes, 0, codes.length);
        req.setCodeFilter(codes);
      }
    }
    RequestMessagesStateResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return resp.getReqId();
  }

  /**
   * Возвращает следующую часть сообщений по идентификатору запроса
   *
   * @param reqId     идентификатор запроса
   * @param pieceSize максимальное кол-во извлекаемых сообщений
   * @param delivery  рассылка
   * @param messages  куда следуют сложить сообщения
   * @return есть ли ещё сообщения
   * @throws AdminException ошибка выполнения команды
   */
  public boolean getNextMessages(int reqId, int pieceSize, Delivery delivery, Collection<mobi.eyeline.informer.admin.delivery.MessageInfo> messages) throws AdminException {
    GetNextMessagesPack req = new GetNextMessagesPack();
    req.setReqId(reqId);
    req.setCount(pieceSize);
    GetNextMessagesPackResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    if (resp.getInfo() != null) {
      for (mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageInfo mi : resp.getInfo()) {
        messages.add(DcpConverter.convert(mi,
            delivery.getType() == Delivery.Type.SingleText ? delivery.getSingleText() : null));
      }
    }
    return resp.getMoreMessages();
  }


  /**
   * Подсчёт кол-ва сообщений
   *
   * @param filter фильтр
   * @return кол-во сообщений
   * @throws AdminException ошибка выполнения команды
   */
  public int countMessages(MessageFilter filter) throws AdminException {
    CountMessages req = new CountMessages();
    if (filter != null) {
      if (filter.getDeliveryId() != null) {
        req.setDeliveryId(filter.getDeliveryId());
      }
      req.setEndDate(DcpConverter.convertDate(filter.getEndDate()));
      req.setStartDate(DcpConverter.convertDate(filter.getStartDate()));

      if (filter.getMsisdnFilter() != null && filter.getMsisdnFilter().length > 0) {
        req.setMsisdnFilter(filter.getMsisdnFilter());
      }
      if (filter.getStates() != null && filter.getStates().length > 0) {
        req.setStates(DcpConverter.convert(filter.getStates()));
      }
      if (filter.getErrorCodes() != null && filter.getErrorCodes().length > 0) {
        int[] codes = new int[filter.getErrorCodes().length];
        System.arraycopy(filter.getErrorCodes(), 0, codes, 0, codes.length);
        req.setCodeFilter(codes);
      }
    }
    CountMessagesResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return resp.getCount();
  }

  /**
   * Возвращает история статусов рассылки
   *
   * @param deliveryId идентификатор рассылки
   * @return история статусов рассылки
   * @throws AdminException ошибка выполнения команды
   */
  public DeliveryStatusHistory getDeliveryHistory(int deliveryId) throws AdminException {
    GetDeliveryHistory req = new GetDeliveryHistory();
    req.setDeliveryId(deliveryId);
    GetDeliveryHistoryResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return DcpConverter.convert(deliveryId, resp.getHistory());
  }

}
