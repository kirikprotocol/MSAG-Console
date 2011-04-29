package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Day;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * @author Aleksandr Khalitov
 */
public class TestDcpConnection extends DcpConnection{

  private static final Logger logger = Logger.getLogger(TestDcpConnection.class);

  private int dIdCounter;

  private long mIdCounter;

  private int reqIds = 0;

  private Map<Integer, Delivery> deliveries = new LinkedHashMap<Integer, Delivery>();

  private Map<Integer, List<Message>> messages = new HashMap<Integer, List<Message>>();          //todo В информере одно сообщение хранится в нескольких статусах. К примеру, new, process, delivered. Надо и здесь сделать также!!

  private Map<Integer, DeliveryStatusHistory> histories = new HashMap<Integer, DeliveryStatusHistory>();

  private Map<Integer, DeliveryRequest> deliveryReqs = new HashMap<Integer, DeliveryRequest>();

  private Map<Integer, MessageRequest> messReqs = new HashMap<Integer, MessageRequest>();

  private ScheduledExecutorService executor;

  public TestDcpConnection() {
    executor = Executors.newSingleThreadScheduledExecutor();
    executor.scheduleWithFixedDelay(new Runnable() {
      public void run() {
        try{
          modifyAll();
        }catch (Exception e){
          logger.error(e,e);
          e.printStackTrace();
        }
      }
    }, 30, 30, TimeUnit.SECONDS);
  }

  public boolean isConnected() {
    return true;
  }

  public synchronized int createDelivery(Delivery delivery) throws AdminException {
    int id = dIdCounter++;
    delivery.setId(id);
    deliveries.put(id, delivery.cloneDelivery());
    histories.put(id, new DeliveryStatusHistory(id,
        new LinkedList<DeliveryStatusHistory.Item>(){{add(new DeliveryStatusHistory.Item(new Date(), DeliveryStatus.Planned));}}));
    return id;
  }

  public synchronized long[] addDeliveryMessages(int deliveryId, List<Message> messages) throws AdminException {
    if(!deliveries.containsKey(deliveryId)) {
      throw new DeliveryException("interaction_error","");
    }
    List<Message> ms = this.messages.get(deliveryId);
    if(ms == null) {
      ms = new LinkedList<Message>();
      this.messages.put(deliveryId, ms);
    }
    long[] ids = new long[messages.size()];
    int i = 0;
    for(Message m : messages) {
      ids[i] = mIdCounter++;
      m.setId(ids[i]);
      m.setState(MessageState.New);
      m.setDate(new Date());
      ms.add(m.cloneMessage());
      i++;
    }
    return ids;
  }

  public void modifyDeliveryGlossary(int deliveryId, String... messages) throws AdminException {
    deliveries.get(deliveryId).setSingleText(messages[0]);
  }

  public String[] getDeliveryGlossary(int deliveryId) throws AdminException {
    return new String[]{deliveries.get(deliveryId).getSingleText()};
  }

  public synchronized void modifyDelivery(Delivery delivery) throws AdminException {
    if(delivery.getId() == null || !deliveries.containsKey(delivery.getId())) {
      throw new DeliveryException("interaction_error","");
    }
    Delivery d = delivery.cloneDelivery();
    d.setDeliveryManager(null);
    d.setStatus(deliveries.get(delivery.getId()).getStatus());
    deliveries.put(delivery.getId(), d);
  }

  public synchronized void dropDelivery(int deliveryId) throws AdminException {
    deliveries.remove(deliveryId);
  }

  @Override
  public void archivateDelivery(int deliveryId) throws AdminException {
    deliveries.remove(deliveryId);
    System.out.println("MOVE DELIVERY TO ARCHIVE: "+deliveryId+" (not implemented)");
  }

  private boolean accept(Delivery delivery, DeliveryFilter filter) {
    if(filter.getEndDateFrom()!= null && delivery.getEndDate() != null && delivery.getEndDate().before(filter.getEndDateFrom())) {
      return false;
    }
    if(filter.getEndDateTo()!= null && delivery.getEndDate() != null && delivery.getEndDate().after(filter.getEndDateTo())) {
      return false;
    }
    if(filter.getNameFilter() != null) {
      boolean accept = false;
      for(String s : filter.getNameFilter()) {
        if(delivery.getName().contains(s)) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }
    if(filter.getStartDateFrom() != null && delivery.getStartDate().before(filter.getStartDateFrom())) {
      return false;
    }
    if(filter.getStartDateTo() != null && delivery.getStartDate().after(filter.getStartDateTo())) {
      return false;
    }
    if(filter.getStatusFilter() != null) {
      boolean accept = false;
      for(DeliveryStatus s : filter.getStatusFilter()) {
        if(s.equals(delivery.getStatus())) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }
    if(filter.getUserIdFilter() != null) {
      boolean accept = false;
      for(String s : filter.getUserIdFilter()) {
        if(s.equals(delivery.getOwner())) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }
    return true;
  }


  private boolean accept(int deliveryId, Message message, MessageFilter filter) {
    if(filter.getDeliveryId() != null && filter.getDeliveryId() != deliveryId) {
      return false;
    }
    if(filter.getMsisdnFilter() != null) {
      boolean accept = false;
      for(String s : filter.getMsisdnFilter()) {
        if(s.equals(message.getAbonent().getSimpleAddress())) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }
    if(filter.getStartDate() != null && message.getDate().before(filter.getStartDate())) {
      return false;
    }
    if(filter.getStates() != null && filter.getStates().length > 0) {
      boolean accept = false;
      for(MessageState s : filter.getStates()) {
        if(s.equals(message.getState())) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }
    if(filter.getEndDate() != null && message.getDate().after(filter.getEndDate())) {
      return false;
    }
    if(filter.getErrorCodes() != null && filter.getErrorCodes().length > 0) {
      boolean accept = false;
      for(Integer s : filter.getErrorCodes()) {
        if(s.equals(message.getErrorCode())) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }

    return true;
  }

  public synchronized int countDeliveries(DeliveryFilter deliveryFilter) throws AdminException {
    int count = 0;
    for(Delivery d : deliveries.values()) {
      if(deliveryFilter != null && !accept(d, deliveryFilter)) {
        continue;
      }
      count++;
    }
    return count;
  }

  public synchronized int countMessages(MessageFilter messageFilter) throws AdminException {
    int count = 0;
    for(Map.Entry<Integer,List<Message>> e : messages.entrySet()) {
      int deliveryId = e.getKey();
      for(Message m : e.getValue()) {
        if(messageFilter != null && !accept(deliveryId, m, messageFilter)) {
          continue;
        }
        count++;
      }
    }
    return count;
  }

  public DeliveryStatusHistory getDeliveryHistory(int deliveryId) throws AdminException {
    return histories.get(deliveryId);
  }

  public synchronized void dropMessages(int deliveryId, long[] messageIds) throws AdminException {
    Set<Long> ids = new HashSet<Long>(messageIds.length);
    for(long m : messageIds) {
      ids.add(m);
    }
    List<Message> ms = messages.get(deliveryId);
    if(ms != null) {
      Iterator<Message> i = ms.iterator();
      while(i.hasNext()) {
        Message m = i.next();
        if(ids.contains(m.getId())) {
          i.remove();
        }
      }
    }
  }

  public Delivery getDelivery(int deliveryId) throws AdminException {
    Delivery d =  deliveries.get(deliveryId);
    return d == null ? null : d.cloneDelivery();
  }

  public synchronized void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException {
    if(!deliveries.containsKey(deliveryId)) {
      throw new DeliveryException("interaction_error","");
    }
    final DeliveryStatus status = state.getStatus();
    final DeliveryStatusHistory oldHistory = histories.get(deliveryId);
    Delivery delivery = deliveries.get(deliveryId);
//    DeliveryStatus current = delivery.getStatus();
//    if(current == status) {
//      return;
//    }
//    switch (status) {
//      case Active:
//        if(current != DeliveryStatus.Planned && current != DeliveryStatus.Paused) {
//          throw new DeliveryException("interaction_error","");
//        }
//        break;
//      case Planned:
//        throw new DeliveryException("interaction_error","");
//      case Cancelled:
//        if(current != DeliveryStatus.Active && current != DeliveryStatus.Paused) {
//          throw new DeliveryException("interaction_error","");
//        }
//        break;
//      case Finished:
//        throw new DeliveryException("interaction_error","");
//      case Paused:
//        if(current != DeliveryStatus.Active) {
//          throw new DeliveryException("interaction_error","");
//        }
//    }
    delivery.setStatus(status);
    histories.put(deliveryId, new DeliveryStatusHistory(deliveryId, new LinkedList<DeliveryStatusHistory.Item>(){{
      addAll(oldHistory.getHistoryItems());
      add(new DeliveryStatusHistory.Item(new Date(), status));}}));
  }

  public synchronized DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
    if(!deliveries.containsKey(deliveryId)) {
      throw new DeliveryException("interaction_error","");
    }
    DeliveryState state = new DeliveryState();
    state.setStatus(deliveries.get(deliveryId).getStatus());
    DeliveryStatistics stats = new DeliveryStatistics();
    List<Message> ms = messages.get(deliveryId);
    int delivered = 0;
    int failed = 0;
    int newD = 0;
    int exp = 0;
    int proc = 0;
    if(ms != null) {
      for(Message m : ms) {
        switch (m.getState()) {
          case Delivered: delivered++; break;
          case New:
            boolean finished = false;
            for(Message _m : ms) {
              if(_m != m && _m.getId().equals(m.getId())) {
                finished = true;
                break;
              }
            }
            if(!finished) {
              newD++;
            }
            break;
          case Failed: failed++; break;
          case Process: exp++; break;
          case Expired: proc++; break;
        }
      }
    }
    stats.setDeliveryState(state);

    stats.setDeliveredMessages(delivered);
    stats.setFailedMessages(failed);
    stats.setExpiredMessages(exp);
    stats.setProcessMessages(proc);
    stats.setNewMessages(newD);
    return stats;
  }

  public synchronized int getDeliveries(DeliveryFilter deliveryFilter) throws AdminException {
    int r = reqIds++;
    deliveryReqs.put(r, new DeliveryRequest(deliveryFilter));
    return r;
  }

  public synchronized boolean getNextDeliveries(int reqId, int pieceSize, Collection<Delivery> deliveries) throws AdminException {
    if(!deliveryReqs.containsKey(reqId) || pieceSize == 0) {
      throw new DeliveryException("interaction_error","");
    }
    DeliveryRequest r = deliveryReqs.get(reqId);
    int count = 0;
    List<Delivery> result = new LinkedList<Delivery>();
    for(Delivery d : this.deliveries.values()) {
      if(accept(d, r.filter) && ++count > r.position) {
        result.add(d);
        if(result.size() == pieceSize) {
          break;
        }
      }
    }
    r.position+=result.size();
    for(Delivery d : result) {
      Delivery info = new Delivery();
      info.setId(d.getId());
      info.setActivePeriodEnd(d.getActivePeriodEnd());
      info.setActivePeriodStart(d.getActivePeriodStart());
      info.setEndDate(d.getEndDate());
      info.setName(d.getName());
      info.setStartDate(d.getStartDate());
      info.setStatus(d.getStatus());
      info.setOwner(d.getOwner());
      for(Map.Entry e : d.getProperties().entrySet()) {
        info.setProperty(e.getKey().toString(), e.getValue().toString());
      }
      deliveries.add(info);
    }
    return result.size() == pieceSize;
  }

  public synchronized int getMessages(MessageFilter filter) throws AdminException {
    int r = reqIds++;
    messReqs.put(r, new MessageRequest(filter));
    return r;
  }

  public synchronized boolean getNextMessages(int reqId, int pieceSize, Collection<Message> messages) throws AdminException {
    MessageRequest req = messReqs.get(reqId);
    if(req == null || pieceSize == 0) {
      throw new DeliveryException("interaction_error","");
    }
    MessageRequest r = messReqs.get(reqId);
    int count = 0;
    List<Message> result = new LinkedList<Message>();
    int deliveryId = req.filter.getDeliveryId();

    if(this.messages.get(deliveryId) == null) {
      return false;
    }

    for(Message m : this.messages.get(deliveryId)) {
      if(accept(deliveryId, m, r.filter) && ++count > r.position) {
        result.add(m);
        if(result.size() == pieceSize) {
          break;
        }
      }
    }

    Delivery delivery = getDelivery(deliveryId);

    r.position+=result.size();
    for(Message d : result) {
      Message info = new Message();
      info.setAbonent(d.getAbonent());
      info.setDate(d.getDate());
      info.setErrorCode(d.getErrorCode());
      if(delivery.getType() == Delivery.Type.SingleText) {
        info.setText(delivery.getSingleText());
      }else {
        info.setText(d.getText());
      }
      info.setState(d.getState());
      info.setId(d.getId());
      for(Map.Entry e : d.getProperties().entrySet()) {
        info.setProperty(e.getKey().toString(), e.getValue().toString());
      }
      messages.add(info);
    }
    return result.size() == pieceSize;
  }

  synchronized void forceActivate() throws AdminException {
    for(Delivery d : deliveries.values()) {
      if(d.getStatus() == DeliveryStatus.Planned || d.getStatus() == null) {
        d.setStatus(DeliveryStatus.Active);
      }
    }
  }


  @SuppressWarnings({"EmptyCatchBlock"})
  synchronized void modifyAll() throws AdminException {
    Random r = new Random();
    SimpleDateFormat sdf = new SimpleDateFormat("HHmmss");
    Date now = new Date();
    for(Delivery d : deliveries.values()) {
      if(d.getStatus() == DeliveryStatus.Planned || d.getStatus() == null) {
        d.setStatus(DeliveryStatus.Active);
      }else if(d.getStatus() != DeliveryStatus.Active) {
        continue;
      }
      if(d.getStartDate().after(now)) {
        continue;
      }
      if(d.getEndDate() != null && d.getEndDate().before(now)) {
        d.setStatus(DeliveryStatus.Finished);
        continue;
      }
      int today = Calendar.getInstance().get(Calendar.DAY_OF_WEEK);
      boolean send = false;
      for(Day day : d.getActiveWeekDays()) {
        if((day.getDay()%7) + 1 == today) {
          send = true;
          break;
        }
      }
      if(!send) {
        continue;
      }
      if(d.getActivePeriodStart().getTimeDate().after(now)) {
        continue;
      }
      if(d.getActivePeriodEnd().getTimeDate().before(now)) {
        continue;
      }
      List<Message> copy = null;
      List<Message> ms = messages.get(d.getId());
      if(ms == null) {
        continue;
      }else {
        copy = new ArrayList<Message>(ms);
      }
      int count = 0;
      for(Message m : copy) {
        if(m.getState() == MessageState.New) {
          if(count<100) {
            m = m.cloneMessage();
            ms.add(m);
            boolean delivered = r.nextBoolean();
            if(delivered) {
              m.setState(MessageState.Delivered);
            }else {
              m.setState(MessageState.Failed);
              m.setErrorCode(1179);
            }
            m.setDate(new Date());
          }
          count++;
        }

      }
      if(count <= 10 ) {
        if(logger.isDebugEnabled()) {
          logger.debug("Delivery is finished: "+d.getName());
        }
        final DeliveryStatusHistory oldHistory = histories.get(d.getId());
        d.setStatus(DeliveryStatus.Finished);
        histories.put(d.getId(), new DeliveryStatusHistory(d.getId(), new LinkedList<DeliveryStatusHistory.Item>(){{
          addAll(oldHistory.getHistoryItems());
          add(new DeliveryStatusHistory.Item(new Date(), DeliveryStatus.Finished));
        }}));
      }

    }
  }

  public void close() {
    if(executor != null) {
      executor.shutdown();
    }
  }










  private static class DeliveryRequest {
    private int position = 0;
    private DeliveryFilter filter;
    private DeliveryRequest(DeliveryFilter filter) throws AdminException {
      this.filter = new DeliveryFilter();
      this.filter.setEndDateFrom(filter.getEndDateFrom());
      this.filter.setEndDateTo(filter.getEndDateTo());
      this.filter.setNameFilter(filter.getNameFilter());
      this.filter.setStartDateFrom(filter.getStartDateFrom());
      this.filter.setStartDateTo(filter.getStartDateTo());
      this.filter.setUserIdFilter(filter.getUserIdFilter());
      this.filter.setStatusFilter(filter.getStatusFilter());
    }
  }

  private static class MessageRequest {
    private int position = 0;
    private MessageFilter filter;
    private MessageRequest(MessageFilter filter) throws AdminException {
      this.filter = new MessageFilter(filter.getDeliveryId(), filter.getStartDate(), filter.getEndDate());
      this.filter.setStates(filter.getStates());
      this.filter.setMsisdnFilter(filter.getMsisdnFilter());
      this.filter.setErrorCodes(filter.getErrorCodes());
    }
  }








}
