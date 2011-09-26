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

  private Map<Integer, List<Message>> messages = new HashMap<Integer, List<Message>>();

  private Map<Integer, DeliveryStatusHistory> histories = new HashMap<Integer, DeliveryStatusHistory>();

  private Map<Integer, DeliveryRequest> deliveryReqs = new HashMap<Integer, DeliveryRequest>();

  private Map<Integer, MessageRequest> messReqs = new HashMap<Integer, MessageRequest>();

  private ScheduledExecutorService executor;

  private int smsPerModify = 50;        //todo

  public TestDcpConnection(boolean daemon, int smsPerModify) {
    if(daemon) {
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
  }

  public TestDcpConnection() {
    this(true, 50);
  }

  public boolean isConnected() {
    return true;
  }

  public synchronized int createDelivery(Delivery delivery) throws AdminException {
    int id = dIdCounter++;
    delivery.setId(id);
    delivery.setCreateDate(new Date());
    delivery.setStatus(DeliveryStatus.Paused);
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
    addHistoryItem(d, d.getStatus());
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
    int r = reqIds++;
    deliveryReqs.put(r, new DeliveryRequest(deliveryFilter));
    return r;
  }

  public synchronized int countMessages(MessageFilter messageFilter) throws AdminException {
    int r = reqIds++;
    messReqs.put(r, new MessageRequest(messageFilter));
    return r;
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
    addHistoryItem(delivery,  status);
  }

  public synchronized DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
    if(!deliveries.containsKey(deliveryId)) {
      throw new DeliveryException("interaction_error","");
    }
    DeliveryState state = new DeliveryState();
    state.setStatus(deliveries.get(deliveryId).getStatus());
    DeliveryStatistics stats = new DeliveryStatistics();
    List<Message> ms = messages.get(deliveryId);
    Set<Long> delivered = new HashSet<Long>();
    Set<Long> failed = new HashSet<Long>();
    Set<Long> newD = new HashSet<Long>();
    Set<Long> exp = new HashSet<Long>();
    Set<Long> proc = new HashSet<Long>();
    Set<Long> retry = new HashSet<Long>();
    Set<Long> sent = new HashSet<Long>();
    if(ms != null) {
      for(Message m : ms) {
        Long id = m.getId();
        switch (m.getState()) {
          case Delivered:
            newD.remove(id);proc.remove(id);sent.remove(id);retry.remove(id);
            delivered.add(id); break;
          case New:
            newD.add(id);
            break;
          case Failed:
            newD.remove(id);proc.remove(id);sent.remove(id);retry.remove(id);
            failed.add(id); break;
          case Process:
            newD.remove(id);proc.add(id);break;
          case Expired:
            newD.remove(id);proc.remove(id);sent.remove(id);retry.remove(id);
            exp.add(id);; break;
          case Sent:
            newD.remove(id);proc.remove(id);sent.add(id);retry.remove(id);
            break;
          case Retry:
            newD.remove(id);proc.remove(id);sent.remove(id);retry.add(id);
            break;
        }
      }
    }
    stats.setDeliveryState(state);

    stats.setDeliveredMessages(delivered.size());
    stats.setFailedMessages(failed.size());
    stats.setExpiredMessages(exp.size());
    stats.setProcessMessages(proc.size());
    stats.setNewMessages(newD.size());
    stats.setRetriedMessages(retry.size());
    stats.setSentMessages(sent.size());
    return stats;
  }

  @Override
  public boolean getNextDeliveriesCount(int reqId, int pieceSize, int[] result) throws AdminException {
    if(!deliveryReqs.containsKey(reqId) || pieceSize == 0) {
      throw new DeliveryException("interaction_error","");
    }
    DeliveryRequest r = deliveryReqs.get(reqId);
    int count = 0;
    int _result = 0;
    for(Delivery d : this.deliveries.values()) {
      if(accept(d, r.filter) && ++count > r.position) {
        _result++;
        if(_result == pieceSize) {
          break;
        }
      }
    }
    r.position+=_result;
    result[0] = _result;
    return _result == pieceSize;
  }

  @Override
  public boolean getNextMessagesCount(int reqId, int pieceSize, int[] result) throws AdminException {
    MessageRequest req = messReqs.get(reqId);
    if(req == null || pieceSize == 0) {
      throw new DeliveryException("interaction_error","");
    }
    MessageRequest r = messReqs.get(reqId);
    int count = 0;
    int _result = 0;
    int deliveryId = req.filter.getDeliveryId();

    if(this.messages.get(deliveryId) == null) {
      return false;
    }

    for(Message m : this.messages.get(deliveryId)) {
      if(accept(deliveryId, m, r.filter) && ++count > r.position) {
        _result++;
        if(_result == pieceSize) {
          break;
        }
      }
    }

    r.position+=_result;
    result[0] = _result;
    return _result == pieceSize;
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
      info.setCreateDate(d.getCreateDate());
      info.setBoundToLocalTime(d.isBoundToLocalTime());
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

  protected synchronized int getMessagesWithFields(MessageFilter filter, MessageField ... fields) throws AdminException {
    return getMessages(filter);
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
      info.setKeywords(d.getKeywords());
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
        addHistoryItem(d, DeliveryStatus.Active);
      }
    }
  }

  synchronized void forceDeliveryFinalization(int deliveryId) throws AdminException {
    Delivery d = deliveries.get(deliveryId);
    if (d == null)
      return;

    List<Message> ms = messages.get(deliveryId);
    for (Message m : ms)
      m.setState(MessageState.Delivered);

    d.setStatus(DeliveryStatus.Finished);
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  synchronized void modifyAll() throws AdminException {
    Random r = new Random();
    Date now = new Date();
    for(Delivery d : deliveries.values()) {
      if(d.getStatus() == DeliveryStatus.Planned || d.getStatus() == null) {
        d.setStatus(DeliveryStatus.Active);
        addHistoryItem(d, DeliveryStatus.Active);
      }else if(d.getStatus() != DeliveryStatus.Active) {
        continue;
      }
      if(d.getStartDate().after(now)) {
        continue;
      }
      if(d.getEndDate() != null && d.getEndDate().before(now)) {
        d.setStatus(DeliveryStatus.Finished);
        addHistoryItem(d, DeliveryStatus.Finished);
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
      if(d.getActivePeriodEnd().getTimeDate().before(now) && !(new SimpleDateFormat("HH:mm:ss").format(now).equals("23:59:59") && d.getActivePeriodEnd().getTimeString().equals("23:59:59"))) {
        continue;
      }
      List<Message> copy = null;
      List<Message> ms = messages.get(d.getId());
      if(ms == null) {
        ms = Collections.emptyList();
      }

      copy = new ArrayList<Message>(ms);

      int count = 0;
      Map<Long, Message> toModify = new HashMap<Long, Message>();
      for(Message m : copy) {
        switch (m.state) {
          case Delivered:
          case Failed:
          case Expired:toModify.remove(m.getId()); break;
//          case Retry: return;
          default: toModify.put(m.getId(), m);
        }
      }
      for(Message m : toModify.values()) {
        if(count< smsPerModify) {
          m = m.cloneMessage();
          ms.add(m);
          int rI= r.nextInt(10);
          switch (rI) {
            case 0:
            case 1:
//              if(m.getState() == MessageState.Sent) {
//                m.setState(MessageState.Retry);
//              }else {
//                m.setState(MessageState.Sent);
//              }
//              break;
            case 2: m.setState(MessageState.Failed); m.setErrorCode(1179); break;
            default: m.setState(MessageState.Delivered); break;
          }
          m.setDate(new Date());
        }
        count++;
      }
      DeliveryStatistics statistics = getDeliveryState(d.getId());
      boolean finished = statistics.getNewMessages() == 0 && statistics.getProcessMessages() == 0 && statistics.getRetriedMessages() == 0 && statistics.getSentMessages() == 0;
      if(finished) {
        if(logger.isDebugEnabled()) {
          logger.debug("Delivery is finished: "+d.getName());
        }
        d.setStatus(DeliveryStatus.Finished);
        addHistoryItem(d, DeliveryStatus.Finished);
      }
    }
  }

  public void close() {
    if(executor != null) {
      executor.shutdown();
    }
  }


  private void addHistoryItem(Delivery d, final DeliveryStatus status) {
    final DeliveryStatusHistory oldHistory = histories.get(d.getId());
    histories.put(d.getId(), new DeliveryStatusHistory(d.getId(), new LinkedList<DeliveryStatusHistory.Item>(){{
      addAll(oldHistory.getHistoryItems());
      add(new DeliveryStatusHistory.Item(new Date(), status));
    }}));
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
