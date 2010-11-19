package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.text.ParseException;
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

  private Map<Integer, DeliveryWStatus> deliveries = new LinkedHashMap<Integer, DeliveryWStatus>();

  private Map<Integer, List<MessageWState>> messages = new HashMap<Integer, List<MessageWState>>();

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
    }, 60, 60, TimeUnit.SECONDS);
  }

  public boolean isConnected() {
    return true;
  }

  public synchronized int createDelivery(Delivery delivery) throws AdminException {
    int id = dIdCounter++;
    deliveries.put(id, new DeliveryWStatus(delivery, id));
    histories.put(id, new DeliveryStatusHistory(id,
        new LinkedList<DeliveryStatusHistory.Item>(){{add(new DeliveryStatusHistory.Item(new Date(), DeliveryStatus.Planned));}}));
    return id;
  }

  public synchronized long[] addDeliveryMessages(int deliveryId, List<Message> messages) throws AdminException {
    if(!deliveries.containsKey(deliveryId)) {
      throw new DeliveryException("interaction_error","");
    }
    List<MessageWState> ms = this.messages.get(deliveryId);
    if(ms == null) {
      ms = new LinkedList<MessageWState>();
      this.messages.put(deliveryId, ms);
    }
    long[] ids = new long[messages.size()];
    int i = 0;
    for(Message m : messages) {
      ids[i] = mIdCounter++;
      m.setId(ids[i]);
      ms.add(new MessageWState(m));
      i++;
    }
    return ids;
  }

  public synchronized long[] addDeliveryAddresses(int deliveryId, List<Address> addresses) throws AdminException {
    if(!deliveries.containsKey(deliveryId)) {
      throw new DeliveryException("interaction_error","");
    }
    List<MessageWState> ms = this.messages.get(deliveryId);
    if(ms == null) {
      ms = new LinkedList<MessageWState>();
      this.messages.put(deliveryId, ms);
    }
    long[] ids = new long[addresses.size()];
    int i = 0;
    for(Address a : addresses) {
      ids[i] = mIdCounter++;
      Message m = Message.newMessage(null);
      m.setAbonent(a);
      m.setId(ids[i]);
      ms.add(new MessageWState(m));
      i++;
    }
    return ids;
  }

  public synchronized void modifyDelivery(Delivery delivery) throws AdminException {
    if(delivery.getId() == null || !deliveries.containsKey(delivery.getId())) {
      throw new DeliveryException("interaction_error","");
    }
    DeliveryWStatus d = new DeliveryWStatus(delivery, delivery.getId());
    d.status = deliveries.get(delivery.getId()).status;
    deliveries.put(delivery.getId(), d);
  }

  public synchronized void dropDelivery(int deliveryId) throws AdminException {
    deliveries.remove(deliveryId);
  }

  private boolean accept(DeliveryWStatus delivery, DeliveryFilter filter) {
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
        if(s.equals(delivery.status)) {
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


  private boolean accept(int deliveryId, MessageWState message, MessageFilter filter) {
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
    if(filter.getStartDate() != null && message.date.before(filter.getStartDate())) {
      return false;
    }
    if(filter.getStates() != null && filter.getStates().length > 0) {
      boolean accept = false;
      for(MessageState s : filter.getStates()) {
        if(s.equals(message.state)) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }
    if(filter.getEndDate() != null && message.date.after(filter.getEndDate())) {
      return false;
    }
    if(filter.getErrorCodes() != null && filter.getErrorCodes().length > 0) {
      boolean accept = false;
      for(Integer s : filter.getErrorCodes()) {
        if(s.equals(message.errorCode)) {
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
    for(DeliveryWStatus d : deliveries.values()) {
      if(deliveryFilter != null && !accept(d, deliveryFilter)) {
        continue;
      }
      count++;
    }
    return count;
  }

  public synchronized int countMessages(MessageFilter messageFilter) throws AdminException {
    int count = 0;
    for(Map.Entry<Integer,List<MessageWState>> e : messages.entrySet()) {
      int deliveryId = e.getKey();
      for(MessageWState m : e.getValue()) {
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
    List<MessageWState> ms = messages.get(deliveryId);
    if(ms != null) {
      Iterator<MessageWState> i = ms.iterator();
      while(i.hasNext()) {
        MessageWState m = i.next();
        if(ids.contains(m.getId())) {
          i.remove();
        }
      }
    }
  }

  public Delivery getDelivery(int deliveryId) throws AdminException {
    DeliveryWStatus d =  deliveries.get(deliveryId);
    return d == null ? null : d.delivery.cloneDelivery();
  }

  public synchronized void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException {
    if(!deliveries.containsKey(deliveryId)) {
      throw new DeliveryException("interaction_error","");
    }
    final DeliveryStatus status = state.getStatus();
    final DeliveryStatusHistory oldHistory = histories.get(deliveryId);
    DeliveryWStatus delivery = deliveries.get(deliveryId);
    DeliveryStatus current = delivery.status;
    if(current == status) {
      return;
    }
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
    delivery.status = status;
    histories.put(deliveryId, new DeliveryStatusHistory(deliveryId, new LinkedList<DeliveryStatusHistory.Item>(){{
      addAll(oldHistory.getHistoryItems());
      add(new DeliveryStatusHistory.Item(new Date(), status));}}));
  }

  public synchronized DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
    if(!deliveries.containsKey(deliveryId)) {
      throw new DeliveryException("interaction_error","");
    }
    DeliveryState state = new DeliveryState();
    state.setStatus(deliveries.get(deliveryId).status);
    DeliveryStatistics stats = new DeliveryStatistics();
    List<MessageWState> ms = messages.get(deliveryId);
    int delivered = 0;
    int failed = 0;
    int newD = 0;
    if(ms != null) {
      for(MessageWState m : ms) {
        switch (m.state) {
          case Delivered: delivered++; break;
          case New: newD++;break;
          case Failed: failed++;
        }
      }
    }
    stats.setDeliveryState(state);
    stats.setDeliveredMessages(delivered);
    stats.setFailedMessages(failed);
    stats.setNewMessages(newD);
    return stats;
  }

  public synchronized int getDeliveries(DeliveryFilter deliveryFilter) throws AdminException {
    int r = reqIds++;
    deliveryReqs.put(r, new DeliveryRequest(deliveryFilter));
    return r;
  }

  public synchronized boolean getNextDeliviries(int reqId, int pieceSize, Collection<DeliveryInfo> deliveries) throws AdminException {
    if(!deliveryReqs.containsKey(reqId) || pieceSize == 0) {
      throw new DeliveryException("interaction_error","");
    }
    DeliveryRequest r = deliveryReqs.get(reqId);
    int count = 0;
    List<DeliveryWStatus> result = new LinkedList<DeliveryWStatus>();
    for(DeliveryWStatus d : this.deliveries.values()) {
      if(accept(d, r.filter) && ++count > r.position) {
        result.add(d);
        if(result.size() == pieceSize) {
          break;
        }
      }
    }
    r.position+=result.size();
    for(DeliveryWStatus d : result) {
      DeliveryInfo info = new DeliveryInfo();
      info.setDeliveryId(d.getId());
      info.setActivityPeriodEnd(d.getActivePeriodEnd());
      info.setActivityPeriodStart(d.getActivePeriodStart());
      info.setEndDate(d.getEndDate());
      info.setName(d.getName());
      info.setStartDate(d.getStartDate());
      info.setStatus(d.status);
      info.setUserId(d.getOwner());
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

  public synchronized boolean getNextMessages(int reqId, int pieceSize, Delivery delivery, Collection<MessageInfo> messages) throws AdminException {
    if(!messReqs.containsKey(reqId) || pieceSize == 0) {
      throw new DeliveryException("interaction_error","");
    }
    MessageRequest r = messReqs.get(reqId);
    int count = 0;
    List<MessageWState> result = new LinkedList<MessageWState>();
    int deliveryId = delivery.getId();

    if(this.messages.get(delivery.getId()) == null) {
      return false;
    }
    
    for(MessageWState m : this.messages.get(delivery.getId())) {
      if(accept(deliveryId, m, r.filter) && ++count > r.position) {
        result.add(m);
        if(result.size() == pieceSize) {
          break;
        }
      }
    }

    r.position+=result.size();
    for(MessageWState d : result) {
      MessageInfo info = new MessageInfo();
      info.setAbonent(d.getAbonent().getSimpleAddress());
      info.setDate(d.date);
      info.setErrorCode(d.errorCode);
      if(delivery.getType() == Delivery.Type.SingleText) {
        info.setText(delivery.getSingleText());
      }else {
        info.setText(d.getText());
      }
      info.setState(d.state);
      info.setId(d.getId());
      for(Map.Entry e : d.getProperties().entrySet()) {
        info.setProperty(e.getKey().toString(), e.getValue().toString());
      }
      messages.add(info);
    }
    return result.size() == pieceSize;
  }


  @SuppressWarnings({"EmptyCatchBlock"})
  private synchronized void modifyAll() throws AdminException {
    Random r = new Random();
    SimpleDateFormat sdf = new SimpleDateFormat("HHmmss");
    Date now = new Date();
    for(DeliveryWStatus d : deliveries.values()) {
      if(d.status == DeliveryStatus.Active) {
        if(d.getStartDate().after(now)) {
          continue;
        }
        if(d.getEndDate() != null && d.getEndDate().before(now)) {
          d.status = DeliveryStatus.Finished;
          continue;
        }
        int today = Calendar.getInstance().get(Calendar.DAY_OF_WEEK);
        boolean send = false;
        for(Delivery.Day day : d.getActiveWeekDays()) {
          if((day.getDay()%7) + 1 == today) {
            send = true;
            break;
          }
        }
        if(!send) {
          continue;
        }
        try{
          if(sdf.parse(sdf.format(d.getActivePeriodStart())).after(sdf.parse(sdf.format(now)))) {
            continue;
          }
          if(sdf.parse(sdf.format(d.getActivePeriodEnd())).before(sdf.parse(sdf.format(now)))) {
            continue;
          }
        }catch (ParseException e){}
        List<MessageWState> ms = messages.get(d.getId());
        if(ms == null) {
          continue;
        }
        int count = 0;
        for(MessageWState m : ms) {
          if(m.state == MessageState.New) {
            if(count<100) {
              boolean delivered = r.nextBoolean();
              if(delivered) {
                m.state = MessageState.Delivered;
              }else {
                m.state = MessageState.Failed;
                m.errorCode = 1179;
              }
              m.date = new Date();
            }
            count++;
          }

        }
        if(count <= 10 ) {
          if(logger.isDebugEnabled()) {
            logger.debug("Delivery is finished: "+d.getName());
          }
          final DeliveryStatusHistory oldHistory = histories.get(d.getId());
          d.status = DeliveryStatus.Finished;
          histories.put(d.getId(), new DeliveryStatusHistory(d.getId(), new LinkedList<DeliveryStatusHistory.Item>(){{
            addAll(oldHistory.getHistoryItems());
            add(new DeliveryStatusHistory.Item(new Date(), DeliveryStatus.Finished));
          }}));
        }
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
      this.filter.setResultFields(filter.getResultFields());
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
      this.filter.setFields(filter.getFields());
      this.filter.setStates(filter.getStates());
      this.filter.setMsisdnFilter(filter.getMsisdnFilter());
      this.filter.setErrorCodes(filter.getErrorCodes());
    }
  }

  private static class DeliveryWStatus extends Delivery {
    private DeliveryStatus status = DeliveryStatus.Planned;
    private Delivery delivery;
    private DeliveryWStatus(Delivery delivery, int id) {
      super(delivery.getType());
      this.delivery = delivery.cloneDelivery();
      this.delivery.setId(id);
    }

    @Override
    public Type getType() {
      return delivery.getType();
    }

    @Override
    public Address getSourceAddress() {
      return delivery.getSourceAddress();
    }

    @Override
    public void setSourceAddress(Address sourceAddress) throws AdminException {
      delivery.setSourceAddress(sourceAddress);
    }


    @Override
    public String getSingleText() {
      return delivery.getSingleText();
    }

    @Override
    public void setSingleText(String singleText) throws AdminException {
      delivery.setSingleText(singleText);
    }

    @Override
    public Integer getId() {
      return delivery.getId();
    }

    @Override
    public void setId(int id) {
      delivery.setId(id);
    }

    @Override
    public String getName() {
      return delivery.getName();
    }

    @Override
    public void setName(String name) throws AdminException {
      delivery.setName(name);
    }

    @Override
    public int getPriority() {
      return delivery.getPriority();
    }

    @Override
    public void setPriority(int priority) throws AdminException {
      delivery.setPriority(priority);
    }

    @Override
    public boolean isTransactionMode() {
      return delivery.isTransactionMode();
    }

    @Override
    public void setTransactionMode(boolean transactionMode) {
      delivery.setTransactionMode(transactionMode);
    }

    @Override
    public Date getStartDate() {
      return delivery.getStartDate();
    }

    @Override
    public void setStartDate(Date startDate) throws AdminException {
      delivery.setStartDate(startDate);
    }

    @Override
    public Date getEndDate() {
      return delivery.getEndDate();
    }

    @Override
    public void setEndDate(Date endDate) {
      delivery.setEndDate(endDate);
    }

    @Override
    public Date getActivePeriodEnd() {
      return delivery.getActivePeriodEnd();
    }

    @Override
    public void setActivePeriodEnd(Date activePeriodEnd) throws AdminException {
      delivery.setActivePeriodEnd(activePeriodEnd);
    }

    @Override
    public Date getActivePeriodStart() {
      return delivery.getActivePeriodStart();
    }

    @Override
    public void setActivePeriodStart(Date activePeriodStart) throws AdminException {
      delivery.setActivePeriodStart(activePeriodStart);
    }

    @Override
    public Day[] getActiveWeekDays() {
      return delivery.getActiveWeekDays();
    }

    @Override
    public void setActiveWeekDays(Day[] days) throws AdminException {
      delivery.setActiveWeekDays(days);
    }

    @Override
    public String getValidityPeriod() {
      return delivery.getValidityPeriod();
    }

    @Override
    public void setValidityPeriod(String validityPeriod) {
      delivery.setValidityPeriod(validityPeriod);
    }

    @Override
    public boolean isFlash() {
      return delivery.isFlash();
    }

    @Override
    public void setFlash(boolean flash) {
      delivery.setFlash(flash);
    }

    @Override
    public boolean isUseDataSm() {
      return delivery.isUseDataSm();
    }

    @Override
    public void setUseDataSm(boolean useDataSm) {
      delivery.setUseDataSm(useDataSm);
    }

    @Override
    public DeliveryMode getDeliveryMode() {
      return delivery.getDeliveryMode();
    }

    @Override
    public void setDeliveryMode(DeliveryMode deliveryMode) throws AdminException {
      delivery.setDeliveryMode(deliveryMode);
    }

    @Override
    public String getOwner() {
      return delivery.getOwner();
    }

    @Override
    public void setOwner(String owner) throws AdminException {
      delivery.setOwner(owner);
    }

    @Override
    public boolean isRetryOnFail() {
      return delivery.isRetryOnFail();
    }

    @Override
    public void setRetryOnFail(boolean retryOnFail) {
      delivery.setRetryOnFail(retryOnFail);
    }

    @Override
    public String getRetryPolicy() {
      return delivery.getRetryPolicy();
    }

    @Override
    public void setRetryPolicy(String retryPolicy) {
      delivery.setRetryPolicy(retryPolicy);
    }

    @Override
    public boolean isReplaceMessage() {
      return delivery.isReplaceMessage();
    }

    @Override
    public void setReplaceMessage(boolean replaceMessage) {
      delivery.setReplaceMessage(replaceMessage);
    }

    @Override
    public String getSvcType() {
      return delivery.getSvcType();
    }

    @Override
    public void setSvcType(String svcType) {
      delivery.setSvcType(svcType);
    }

    @Override
    public void setProperty(String name, String value) {
      delivery.setProperty(name, value);
    }

    @Override
    public void addProperties(Map<String, String> props) {
      delivery.addProperties(props);
    }

    @Override
    public String getProperty(String name) {
      return delivery.getProperty(name);
    }

    @Override
    public boolean containsProperty(String name) {
      return delivery.containsProperty(name);
    }

    public String removeProperty(String name) {
      return delivery.removeProperty(name);
    }

    @Override
    public Properties getProperties() {
      return delivery.getProperties();
    }
  }

  private static class MessageWState extends Message{

    private Message message;

    private MessageState state = MessageState.New;

    private Date date = new Date();

    private Integer errorCode = 0;

    private MessageWState(Message message) {
      this.message = message.cloneMessage();
    }

    @Override
    public Long getId() {
      return message.getId();
    }

    @Override
    public void setId(Long id) {
      message.setId(id);
    }

    @Override
    public Address getAbonent() {
      return message.getAbonent();
    }

    @Override
    public void setAbonent(Address msisdn) throws AdminException {
      message.setAbonent(msisdn);
    }

    @Override
    public String getText() {
      return message.getText();
    }


    @Override
    void setText(String text) {
      message.setText(text);
    }
  }




}
