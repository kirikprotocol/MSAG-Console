package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

/**
 * @author Aleksandr Khalitov
 */
class TestSiebelDataProvider implements SiebelDataProvider{

  private Map<String, List<SiebelMessage>> messages = new ConcurrentHashMap<String, List<SiebelMessage>>();

  private Map<String, SiebelDelivery> deliveries = new ConcurrentHashMap<String, SiebelDelivery>();

  private boolean shutdown = true;

  void addDelivery(SiebelDelivery d) {
    deliveries.put(d.getWaveId(), d);
  }

  synchronized void addMessage(String waveId, SiebelMessage m) {
    List<SiebelMessage> ms = messages.get(waveId);
    if(ms == null) {
      ms = new LinkedList<SiebelMessage>();
      messages.put(waveId, ms);
    }
    ms.add(m);
  }

  public SiebelMessage getMessage(String clcId) throws AdminException {
    for(List<SiebelMessage> ms : messages.values()) {
      for(SiebelMessage s : ms) {
        if(s.getClcId().endsWith(clcId)) {
          return s;
        }
      }
    }
    return null;
  }

  public ResultSet<SiebelMessage> getMessages(String waveId) throws AdminException {
    if(!messages.containsKey(waveId)) {
      return new EmptyResultSet<SiebelMessage>();
    }
    final Iterator<SiebelMessage> i = new ArrayList<SiebelMessage>(messages.get(waveId)).iterator();
    return new ResultSet<SiebelMessage>() {
      public boolean next() throws SiebelException {
        return i.hasNext();
      }
      public SiebelMessage get() throws SiebelException {
        return i.next();
      }
      public void close() {
      }
    };
  }

  public SiebelMessage.State getMessageState(String clcId) throws AdminException {
    for(List<SiebelMessage> ms : messages.values()) {
      for(SiebelMessage s : ms) {
        if(s.getClcId().endsWith(clcId)) {
          return s.getMessageState();
        }
      }
    }
    return null;
  }

  public void setMessageStates(Map<String, SiebelMessage.DeliveryState> deliveryStates) throws AdminException {
    for(List<SiebelMessage> ms : messages.values()) {
      for(SiebelMessage s : ms) {
        if(deliveryStates.containsKey(s.getClcId())) {
          SiebelMessage.DeliveryState state = deliveryStates.get(s.getClcId());
          s.setSmscCode(state.getSmppCode());
          s.setSmscValue(state.getSmppCodeDescription());
          s.setMessageState(state.getState());
          s.setLastUpd(new Date());
        }
      }
    }
  }

  public SiebelDelivery getDelivery(String waveId) throws AdminException {
    return deliveries.get(waveId);
  }

  public ResultSet<SiebelDelivery> getDeliveriesToUpdate(Date from) throws AdminException {
    List<SiebelDelivery> res = new LinkedList<SiebelDelivery>();
    for(SiebelDelivery sd : deliveries.values()) {
      if(sd.getLastUpdate().after(from)) {
        res.add(sd);
      }
    }
    final Iterator<SiebelDelivery> i = res.iterator();
    return new ResultSet<SiebelDelivery>() {
      public boolean next() throws SiebelException {
        return i.hasNext();
      }
      public SiebelDelivery get() throws SiebelException {
        return i.next();
      }
      public void close() {}
    };
  }

  public ResultSet<SiebelDelivery> getDeliveries() throws AdminException {
    final Iterator<SiebelDelivery> i = new ArrayList<SiebelDelivery>(deliveries.values()).iterator();
    return new ResultSet<SiebelDelivery>() {
      public boolean next() throws SiebelException {
        return i.hasNext();
      }
      public SiebelDelivery get() throws SiebelException {
        return i.next();
      }
      public void close() {}
    };
  }

  public void setDeliveryStatus(String waveId, SiebelDelivery.Status status) throws AdminException {
    SiebelDelivery sd = deliveries.get(waveId);
    if(sd != null) {
      sd.setStatus(status);
      sd.setLastUpdate(new Date());
    }
  }

  public void setDeliveryStatuses(Map<String, SiebelDelivery.Status> statuses) throws AdminException {
    for(Map.Entry<String, SiebelDelivery.Status> e : statuses.entrySet()) {
      SiebelDelivery sd = deliveries.get(e.getKey());
      if(sd != null) {
        sd.setStatus(e.getValue());
        sd.setLastUpdate(new Date());
      }
    }
  }

  public SiebelDelivery.Status getDeliveryStatus(String waveId) throws AdminException {
    SiebelDelivery sd = deliveries.get(waveId);
    return sd == null ? null : sd.getStatus();
  }

  public boolean containsUnfinished(String waveId) throws AdminException {
    return false;       //todo
  }

  public void connect(Properties props) throws AdminException {
    shutdown = false;
  }

  public boolean isShutdowned() {
    return shutdown;
  }

  public void shutdown() {
    shutdown = true;
  }

  private static class EmptyResultSet<T> implements ResultSet<T> {
    public boolean next() throws SiebelException {
      return false;
    }

    public T get() throws SiebelException {
      return null;
    }

    public void close() {
    }
  }
}
