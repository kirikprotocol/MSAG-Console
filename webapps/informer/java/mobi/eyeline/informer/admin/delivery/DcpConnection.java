package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.DcpClient;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryFields;
import static mobi.eyeline.informer.admin.delivery.DcpConverter.*;

import java.util.Collection;
import java.util.List;

/**
 * Коннект к DCP, упрвление рассылками
 *
 * @author Aleksandr Khalitov
 */
class DcpConnection {

  private DcpClient client;

  public DcpConnection(String host, int port, final String login, String password) throws AdminException {
    this.client = new DcpClient(host, port, login, password);
  }

  protected DcpConnection() {
  }

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
    req.setInfo(convert(delivery));

    CreateDeliveryResp resp = client.send(req);
    return resp.getDeliveryId();
  }

  public void modifyDeliveryGlossary(int deliveryId, String... messages) throws AdminException {
    ModifyDeliveryGlossary reqG = new ModifyDeliveryGlossary();
    reqG.setDeliveryId(deliveryId);
    DeliveryGlossary glossary = new DeliveryGlossary();
    glossary.setMessages(messages);
    reqG.setGlossary(glossary);
    client.send(reqG);
  }

  public String[] getDeliveryGlossary(int deliveryId) throws AdminException {
    GetDeliveryGlossary reqG = new GetDeliveryGlossary();
    GetDeliveryGlossaryResp respG;
    reqG.setDeliveryId(deliveryId);
    respG = client.send(reqG);
    return respG.getGlossary().getMessages();
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
    req.setMessages(convert(messages));
    AddDeliveryMessagesResp resp = client.send(req);
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
    req.setInfo(convert(delivery));
    client.send(req);
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
    client.send(req);
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
        f.setEndDateFrom(convertDateToDcpFormat(deliveryFilter.getEndDateFrom()));
      }
      if (deliveryFilter.getEndDateTo() != null) {
        f.setEndDateTo(convertDateToDcpFormat(deliveryFilter.getEndDateTo()));
      }
      if (deliveryFilter.getStartDateFrom() != null) {
        f.setStartDateFrom(convertDateToDcpFormat(deliveryFilter.getStartDateFrom()));
      }
      if (deliveryFilter.getStartDateTo() != null) {
        f.setStartDateTo(convertDateToDcpFormat(deliveryFilter.getStartDateTo()));
      }
      if (deliveryFilter.getNameFilter() != null && deliveryFilter.getNameFilter().length > 0) {
        f.setNameFilter(deliveryFilter.getNameFilter());
      }
      if (deliveryFilter.getUserIdFilter() != null && deliveryFilter.getUserIdFilter().length > 0) {
        f.setUserIdFilter(deliveryFilter.getUserIdFilter());
      }
      if (deliveryFilter.getStatusFilter() != null && deliveryFilter.getStatusFilter().length > 0) {
        f.setStatusFilter(convert(deliveryFilter.getStatusFilter()));
      }
    }
    req.setFilter(f);
    CountDeliveriesResp resp = client.send(req);
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
    client.send(req);
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
    GetDeliveryInfoResp resp = client.send(req);
    if (!resp.hasInfo()) {
      return null;
    }
    mobi.eyeline.informer.admin.delivery.protogen.protocol.DeliveryInfo info = resp.getInfo();
    return convert(deliveryId, info);
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
    req.setState(convert(state));
    client.send(req);
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
    resp = client.send(req);
    return convert(resp.getStats(), resp.getState());
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
      f.setEndDateFrom(convertDateToDcpFormat(deliveryFilter.getEndDateFrom()));
    }
    if (deliveryFilter.getStartDateFrom() != null) {
      f.setStartDateFrom(convertDateToDcpFormat(deliveryFilter.getStartDateFrom()));
    }
    if (deliveryFilter.getEndDateTo() != null) {
      f.setEndDateTo(convertDateToDcpFormat(deliveryFilter.getEndDateTo()));
    }
    if (deliveryFilter.getStartDateTo() != null) {
      f.setStartDateTo(convertDateToDcpFormat(deliveryFilter.getStartDateTo()));
    }
    if (deliveryFilter.getEndDateFrom() != null) {
      f.setEndDateFrom(convertDateToDcpFormat(deliveryFilter.getEndDateFrom()));
    }
    if (deliveryFilter.getEndDateTo() != null) {
      f.setEndDateTo(convertDateToDcpFormat(deliveryFilter.getEndDateTo()));
    }
    if (deliveryFilter.getStartDateFrom() != null) {
      f.setStartDateFrom(convertDateToDcpFormat(deliveryFilter.getStartDateFrom()));
    }
    if (deliveryFilter.getStartDateTo() != null) {
      f.setStartDateTo(convertDateToDcpFormat(deliveryFilter.getStartDateTo()));
    }
    if (deliveryFilter.getNameFilter() != null && deliveryFilter.getNameFilter().length > 0) {
      f.setNameFilter(deliveryFilter.getNameFilter());
    }
    if (deliveryFilter.getUserIdFilter() != null && deliveryFilter.getUserIdFilter().length > 0) {
      f.setUserIdFilter(deliveryFilter.getUserIdFilter());
    }
    if (deliveryFilter.getStatusFilter() != null && deliveryFilter.getStatusFilter().length > 0) {
      f.setStatusFilter(convert(deliveryFilter.getStatusFilter()));
    }
    DeliveryFields[] fieldses = DeliveryFields.values();
    DeliveryFields[] fs = new DeliveryFields[fieldses.length + 1];
    fs[0] = DeliveryFields.UserData;
    System.arraycopy(fieldses, 0, fs, 1, fieldses.length);
    req.setResultFields(fs);
    req.setFilter(f);

    GetDeliveriesListResp resp = client.send(req);
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
  public boolean getNextDeliveries(int reqId, int pieceSize, Collection<Delivery> deliveries) throws AdminException {
    GetDeliveriesListNext req = new GetDeliveriesListNext();
    req.setReqId(reqId);
    req.setCount(pieceSize);
    GetDeliveriesListNextResp resp;
    resp = client.send(req);
    if (resp.getInfo() != null) {
      for (DeliveryListInfo di : resp.getInfo()) {
        deliveries.add(convert(di));
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
      req.setEndDate(convertDateToDcpFormat(filter.getEndDate()));
      req.setStartDate(convertDateToDcpFormat(filter.getStartDate()));

      ReqField[] fieldses = ReqField.values();
      ReqField[] fs = new ReqField[fieldses.length + 1];
      fs[0] = ReqField.UserData;
      System.arraycopy(fieldses, 0, fs, 1, fieldses.length);
      req.setFields(fs);

      if (filter.getMsisdnFilter() != null && filter.getMsisdnFilter().length > 0) {
        req.setMsisdnFilter(filter.getMsisdnFilter());
      }
      if (filter.getStates() != null && filter.getStates().length > 0) {
        req.setStates(convert(filter.getStates()));
      }
      if (filter.getErrorCodes() != null && filter.getErrorCodes().length > 0) {
        int[] codes = new int[filter.getErrorCodes().length];
        System.arraycopy(filter.getErrorCodes(), 0, codes, 0, codes.length);
        req.setCodeFilter(codes);
      }
    }
    RequestMessagesStateResp resp = client.send(req);
    return resp.getReqId();
  }

  /**
   * Возвращает следующую часть сообщений по идентификатору запроса
   *
   * @param reqId     идентификатор запроса
   * @param pieceSize максимальное кол-во извлекаемых сообщений
   * @param messages  куда следуют сложить сообщения
   * @return есть ли ещё сообщения
   * @throws AdminException ошибка выполнения команды
   */
  public boolean getNextMessages(int reqId, int pieceSize, Collection<Message> messages) throws AdminException {
    GetNextMessagesPack req = new GetNextMessagesPack();
    req.setReqId(reqId);
    req.setCount(pieceSize);
    GetNextMessagesPackResp resp = client.send(req);
    if (resp.getInfo() != null) {
      for (mobi.eyeline.informer.admin.delivery.protogen.protocol.MessageInfo mi : resp.getInfo()) {
        messages.add(convert(mi));
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
      req.setEndDate(convertDateToDcpFormat(filter.getEndDate()));
      req.setStartDate(convertDateToDcpFormat(filter.getStartDate()));

      if (filter.getMsisdnFilter() != null && filter.getMsisdnFilter().length > 0) {
        req.setMsisdnFilter(filter.getMsisdnFilter());
      }
      if (filter.getStates() != null && filter.getStates().length > 0) {
        req.setStates(convert(filter.getStates()));
      }
      if (filter.getErrorCodes() != null && filter.getErrorCodes().length > 0) {
        int[] codes = new int[filter.getErrorCodes().length];
        System.arraycopy(filter.getErrorCodes(), 0, codes, 0, codes.length);
        req.setCodeFilter(codes);
      }
    }
    CountMessagesResp resp = client.send(req);
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
    GetDeliveryHistoryResp resp = client.send(req);
    return convert(deliveryId, resp.getHistory());
  }

}
