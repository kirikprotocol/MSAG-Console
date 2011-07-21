package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Time;
import org.apache.log4j.Logger;

import java.util.Collection;
import java.util.Date;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;


/**
 * author: Aleksandr Khalitov
 */
public class GetMessagesLightStrategy implements GetMessagesStrategy{

  private static final Logger logger = Logger.getLogger(GetMessagesLightStrategy.class);

  private MessageState[] states = new MessageState[100];

  private final Set<Long> resended = new HashSet<Long>();

  private final String resendProperty;

  public GetMessagesLightStrategy(String resendProperty) {
    this.resendProperty = resendProperty;
  }

  private static MessageState[] extendArray(MessageState[] arr, int toSize) {
    MessageState[] newArr = new MessageState[toSize];
    System.arraycopy(arr, 0, newArr, 0, arr.length);
    return newArr;
  }

  private void putState(long _id, MessageState element) {
    int id = (int)_id;
    if (states.length <= id)
      states = extendArray(states, id + 1000);
    states[id] = element;
  }

  private void loadStatesAndResended(MessageFilter filter, DcpConnection conn, int deliveryId, int _pieceSize) throws AdminException {
    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Load states and resended: id="+deliveryId);
      }
      Delivery d = conn.getDelivery(deliveryId);
      MessageFilter emptyFilter = new MessageFilter(filter);
      emptyFilter.setStartDate(d.getCreateDate());
      emptyFilter.setEndDate(d.getEndDate() != null ? d.getEndDate() : new Date(System.currentTimeMillis()+(24*60*60*1000)));
      emptyFilter.setStates();

      int _reqId = conn.getMessagesWithFields(emptyFilter, MessageField.UserData, MessageField.State);

      new VisitorHelperImpl(_pieceSize, _reqId, conn).visit(new Visitor<Message>() {
        @Override
        public boolean visit(Message _m) throws AdminException {
          String _p =_m.getProperty(resendProperty);
          if(_p != null) {
            resended.add(Long.parseLong(_p));
          }
          putState(_m.getId(), _m.getState());
          return true;
        }
      });
    }finally {
      if(logger.isDebugEnabled()) {
        logger.debug("Loading (id="+deliveryId+") is completed. Time="+(System.currentTimeMillis()-now)+"ms");
      }
    }
  }

  @Override
  public void getMessages(DcpConnection conn, final MessageFilter filter, int _pieceSize, final Visitor<Message> visitor) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Get messages for delivery: id="+filter.getDeliveryId());
    }
    visitMessages(conn, filter, _pieceSize, new Visitor<Message>() {
      @Override
      public boolean visit(Message _m) throws AdminException {
        if(resended.contains(_m.getId())) {
          _m.setResended(true);
        }
        if(filter.isNoResended() && _m.isResended()) {
          return true;
        }
        int id = (int)_m.getId().longValue();
        if(states.length<=id) {
          return true;
        }
        MessageState realState = states[id];
        if(realState != null && realState == _m.getState()) {
          states[id] = null;         // message is processed, remove it from array
          return visitor.visit(_m);
        }
        return true;
      }
    }, MessageField.values());
  }

  @Override
  public int countMessages(DcpConnection conn, final MessageFilter filter) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Count messages for delivery: id="+filter.getDeliveryId());
    }
    final int[] result = new int[]{0};

    visitMessages(conn, filter, 1000, new Visitor<Message>() {
      @Override
      public boolean visit(Message _m) throws AdminException {
        if (filter.isNoResended() && resended.contains(_m.getId())) {
          return true;
        }
        int id = (int) _m.getId().longValue();
        if (states.length <= id) {
          return true;
        }
        MessageState realState = states[id];
        if (realState != null && realState == _m.getState()) {
          states[id] = null;       // message is processed, remove it from array
          result[0]++;
        }
        return true;
      }
    }, MessageField.State);
    return result[0];
  }

  private void visitMessages(DcpConnection conn, final MessageFilter filter, int _pieceSize, Visitor<Message> visitor, MessageField ... fields) throws AdminException {
    loadStatesAndResended(filter, conn, filter.getDeliveryId(), _pieceSize);
    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Visit messages: id="+filter.getDeliveryId());
      }
      int _reqId = conn.getMessagesWithFields(filter, fields);
      new VisitorHelperImpl(_pieceSize, _reqId, conn).visit(visitor);
    }finally {
      if(logger.isDebugEnabled()) {
        logger.debug("Visiting is completed (id="+filter.getDeliveryId()+"). Time="+(System.currentTimeMillis() - now)+"ms");
      }
    }
  }

  private class VisitorHelperImpl extends VisitorHelper<Message> {
    VisitorHelperImpl(int pieceSize, int reqId, DcpConnection dcpConnection) {
      super(pieceSize, reqId, dcpConnection);
    }

    protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<Message> result) throws AdminException {
      return connection.getNextMessages(reqId, pieceSize, result);
    }
  }

  public static void main(String... args) throws AdminException {
    DcpConnection conn = new DcpConnection("silverstone", 9573, "artem", "laefeeza1");
    GetMessagesLightStrategy s = new GetMessagesLightStrategy("resend");
    MessageFilter f = new MessageFilter(1245, new Date(System.currentTimeMillis() - 3600 * 1000 * 24), new Date());
    f.setMsisdnFilter("+79139495113");

    final AtomicInteger count = new AtomicInteger(0);

    long start = System.currentTimeMillis();
    s.getMessages(conn, f, 1000, new Visitor<Message>() {
      public boolean visit(Message value) throws AdminException {
        count.incrementAndGet();
        return false;
      }
    });

    long totalTime =(System.currentTimeMillis() - start);
    System.out.println(totalTime + " ms.");
    conn.close();
  }
}
