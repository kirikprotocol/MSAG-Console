package mobi.eyeline.informer.admin.delivery.protogen;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.delivery.DeliveryState;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;
import org.apache.log4j.Logger;

import java.util.Collection;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */
public class DcpConnectionImpl implements DcpConnection {

  private static final Logger logger = Logger.getLogger(DcpConnectionImpl.class);

  private final DcpClient client;

  private final Lock lock;

  public DcpConnectionImpl(String host, int port, final String login, String password) throws AdminException {
    this.client = new DcpClient(host, port);
    connect(login, password);

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


  private void connect(String login, String password) throws AdminException {
    UserAuth auth = new UserAuth();
    auth.setUserId(login);
    auth.setPassword(password);
    client.send(auth);
  }

  public boolean isConnected() {
    return client.isConnected();
  }

  public void close() {
    client.shutdown();
  }


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
    return resp.getDeliveryId();
  }

  public long[] addDeliveryMessages(int deliveryId, Message[] messages) throws AdminException {
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
  }

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

  public int countDeliveries(DeliveryFilter deliveryFilter) throws AdminException {
    CountDeliveries req = new CountDeliveries();
    if (deliveryFilter != null) {
      if (deliveryFilter.getEndDateFrom() != null) {
        req.setEndDateFrom(DcpConverter.convertDate(deliveryFilter.getEndDateFrom()));
      }
      if (deliveryFilter.getEndDateTo() != null) {
        req.setEndDateTo(DcpConverter.convertDate(deliveryFilter.getEndDateTo()));
      }
      if (deliveryFilter.getStartDateFrom() != null) {
        req.setStartDateFrom(DcpConverter.convertDate(deliveryFilter.getStartDateFrom()));
      }
      if (deliveryFilter.getStartDateTo() != null) {
        req.setStartDateTo(DcpConverter.convertDate(deliveryFilter.getStartDateTo()));
      }
      if (deliveryFilter.getNameFilter() != null && deliveryFilter.getNameFilter().length > 0) {
        req.setNameFilter(deliveryFilter.getNameFilter());
      }
      if (deliveryFilter.getUserIdFilter() != null && deliveryFilter.getUserIdFilter().length > 0) {
        req.setUserIdFilter(deliveryFilter.getUserIdFilter());
      }
      if (deliveryFilter.getStatusFilter() != null && deliveryFilter.getStatusFilter().length > 0) {
        req.setStatusFilter(DcpConverter.convert(deliveryFilter.getStatusFilter()));
      }
    }
    CountDeliveriesResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return resp.getResult();
  }


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

  public void dropMessages(long[] messageIds) throws AdminException {
    DropDeliverymessages req = new DropDeliverymessages();
    req.setMessageIds(messageIds);
    try {
      lock.lock();
      client.send(req);
    } finally {
      lock.unlock();
    }
  }

  public String[] getDeliveryGlossary(int deliveryId) throws AdminException {
    GetDeliveryGlossary req = new GetDeliveryGlossary();
    req.setDeliveryId(deliveryId);
    GetDeliveryGlossaryResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return resp.getGlossary().getMessages();
  }

  public void modifyDeliveryGlossary(int deliveryId, String[] messages) throws AdminException {
    ModifyDeliveryGlossary req = new ModifyDeliveryGlossary();
    req.setDeliveryId(deliveryId);
    DeliveryGlossary glossary = new DeliveryGlossary();
    glossary.setMessages(messages);
    req.setGlossary(glossary);
    try {
      lock.lock();
      client.send(req);
    } finally {
      lock.unlock();
    }
  }


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
    return DcpConverter.convert(deliveryId, resp.getInfo());
  }


  public int getDeliviries(DeliveryFilter deliveryFilter) throws AdminException {
    GetDeliveriesList req = new GetDeliveriesList();
    if (deliveryFilter.getEndDateFrom() != null) {
      req.setEndDateFrom(DcpConverter.convertDate(deliveryFilter.getEndDateFrom()));
    }
    if (deliveryFilter.getStartDateFrom() != null) {
      req.setStartDateFrom(DcpConverter.convertDate(deliveryFilter.getStartDateFrom()));
    }
    if (deliveryFilter.getEndDateTo() != null) {
      req.setEndDateTo(DcpConverter.convertDate(deliveryFilter.getEndDateTo()));
    }
    if (deliveryFilter.getStartDateTo() != null) {
      req.setStartDateTo(DcpConverter.convertDate(deliveryFilter.getStartDateTo()));
    }
    if (deliveryFilter.getEndDateFrom() != null) {
      req.setEndDateFrom(DcpConverter.convertDate(deliveryFilter.getEndDateFrom()));
    }
    if (deliveryFilter.getEndDateTo() != null) {
      req.setEndDateTo(DcpConverter.convertDate(deliveryFilter.getEndDateTo()));
    }
    if (deliveryFilter.getStartDateFrom() != null) {
      req.setStartDateFrom(DcpConverter.convertDate(deliveryFilter.getStartDateFrom()));
    }
    if (deliveryFilter.getStartDateTo() != null) {
      req.setStartDateTo(DcpConverter.convertDate(deliveryFilter.getStartDateTo()));
    }
    if (deliveryFilter.getNameFilter() != null && deliveryFilter.getNameFilter().length > 0) {
      req.setNameFilter(deliveryFilter.getNameFilter());
    }
    if (deliveryFilter.getUserIdFilter() != null && deliveryFilter.getUserIdFilter().length > 0) {
      req.setUserIdFilter(deliveryFilter.getUserIdFilter());
    }
    if (deliveryFilter.getStatusFilter() != null && deliveryFilter.getStatusFilter().length > 0) {
      req.setStatusFilter(DcpConverter.convert(deliveryFilter.getStatusFilter()));
    }
    GetDeliveriesListResp resp;
    try {
      lock.lock();
      resp = client.send(req);
    } finally {
      lock.unlock();
    }
    return resp.getReqId();
  }

  public boolean getNextDeliviries(int reqId, int pieceSize, Collection<mobi.eyeline.informer.admin.delivery.DeliveryInfo> deliveries) throws AdminException {
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

  public int getMessagesStates(MessageFilter filter) throws AdminException {
    RequestMessagesState req = new RequestMessagesState();
    if (filter != null) {
      if (filter.getDeliveryId() != null) {
        req.setDeliveryId(filter.getDeliveryId());
      }
      req.setEndDate(DcpConverter.convertDate(filter.getEndDate()));
      req.setStartDate(DcpConverter.convertDate(filter.getStartDate()));

      if (filter.getFields() != null && filter.getFields().length > 0) {
        req.setFields(DcpConverter.convert(filter.getFields()));
      }
      if (filter.getMsisdnFilter() != null && filter.getMsisdnFilter().length > 0) {
        req.setMsisdnFilter(filter.getMsisdnFilter());
      }
      if (filter.getStates() != null && filter.getStates().length > 0) {
        req.setStates(DcpConverter.convert(filter.getStates()));
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

  public boolean getNextMessageStates(int reqId, int pieceSize, Collection<mobi.eyeline.informer.admin.delivery.MessageInfo> messages) throws AdminException {
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
        messages.add(DcpConverter.convert(mi));
      }
    }
    return resp.getMoreMessages();
  }


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
}
