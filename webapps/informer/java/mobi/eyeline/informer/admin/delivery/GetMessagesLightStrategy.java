package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Logger;

import java.util.*;


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

  private EnumMap<MessageState, Object> buildStatesMap(MessageFilter filter) {
    final EnumMap<MessageState, Object> statesMap = new EnumMap<MessageState, Object>(MessageState.class);
    if (filter.getStates() != null) {
      for (MessageState s : filter.getStates())
        statesMap.put(s, null);
    } else {
      for (MessageState s : MessageState.values())
        statesMap.put(s, null);
    }
    return statesMap;
  }


  private void loadStatesAndResended(DcpConnection conn, int deliveryId, int _pieceSize) throws AdminException {
    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Load states and resended: id="+deliveryId);
      }
      Delivery d = conn.getDelivery(deliveryId);
      MessageFilter emptyFilter = new MessageFilter(d.getId(), d.getCreateDate(), d.getEndDate() != null ? d.getEndDate() : new Date(System.currentTimeMillis()+(24*60*60*1000)));
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

    loadStatesAndResended(conn, filter.getDeliveryId(), _pieceSize);

    final EnumMap<MessageState, Object> statesMap = buildStatesMap(filter);

    final MessageFilter filterWOState = new MessageFilter(filter);
    filterWOState.setStates(new MessageState[0]);


    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Process messages: id="+filter.getDeliveryId());
      }
      int _reqId = conn.getMessages(filterWOState);
      new VisitorHelperImpl(_pieceSize, _reqId, conn).visit(new Visitor<Message>() {
        @Override
        public boolean visit(Message _m) throws AdminException {
          if(resended.contains(_m.getId())) {
            _m.setResended(true);
          }
          if(filterWOState.isNoResended() && _m.isResended()) {
            return true;
          }
          int id = (int)_m.getId().longValue();
          if(states.length<=id) {
            return true;
          }
          MessageState realState = states[id];
          if(realState != null && realState == _m.getState()) {
            if(statesMap.containsKey(realState)) {
              states[id] = null;         // message is processed, remove it from array
              return visitor.visit(_m);
            }
          }
          return true;
        }
      });
    }finally {
      if(logger.isDebugEnabled()) {
        logger.debug("Processing is completed (id="+filter.getDeliveryId()+"). Time="+(System.currentTimeMillis() - now)+"ms");
      }

    }
  }

  @Override
  public int countMessages(DcpConnection conn, MessageFilter filter) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Count messages for delivery: id="+filter.getDeliveryId());
    }

    loadStatesAndResended(conn, filter.getDeliveryId(), 1000);

    final EnumMap<MessageState, Object> statesMap = buildStatesMap(filter);

    final MessageFilter filterWOState = new MessageFilter(filter);
    filterWOState.setStates(new MessageState[0]);

    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Process messages: id="+filter.getDeliveryId());
      }
      int _reqId = conn.getMessagesWithFields(filterWOState, MessageField.State);
      final int[] result = new int[]{0};

      new VisitorHelperImpl(1000, _reqId, conn).visit(new Visitor<Message>() {
        @Override
        public boolean visit(Message _m) throws AdminException {
          if(filterWOState.isNoResended() && resended.contains(_m.getId())) {
            return true;
          }
          int id = (int)_m.getId().longValue();
          if(states.length<=id) {
            return true;
          }
          MessageState realState = states[id];
          if(realState != null && realState == _m.getState()) {
            if(statesMap.containsKey(realState)) {
              states[id] = null;       // message is processed, remove it from array
              result[0]++;
            }
          }
          return true;
        }
      });
      return result[0];
    }finally {
      if(logger.isDebugEnabled()) {
        logger.debug("Processing is completed (id="+filter.getDeliveryId()+"). Time="+(System.currentTimeMillis() - now)+"ms");
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
}
