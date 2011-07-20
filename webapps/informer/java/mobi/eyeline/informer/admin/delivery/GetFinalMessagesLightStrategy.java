package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Logger;

import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class GetFinalMessagesLightStrategy implements GetMessagesStrategy{

  private static final Logger logger = Logger.getLogger(GetFinalMessagesLightStrategy.class);

  private final String resendProperty;

  private final Set<Long> resended = new HashSet<Long>();


  GetFinalMessagesLightStrategy(String resendProperty) {
    this.resendProperty = resendProperty;
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


  private void loadResended(DcpConnection conn, int deliveryId, int _pieceSize) throws AdminException {
    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Load resended: id="+deliveryId);
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

    loadResended(conn, filter.getDeliveryId(), _pieceSize);

    final EnumMap<MessageState, Object> statesMap = buildStatesMap(filter);


    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Process messages: id="+filter.getDeliveryId());
      }
      int _reqId = conn.getMessages(filter);

      new VisitorHelperImpl(_pieceSize, _reqId, conn).visit(new Visitor<Message>() {
        @Override
        public boolean visit(Message _m) throws AdminException {
          if(resended.contains(_m.getId())) {
            _m.setResended(true);
          }
          if(filter.isNoResended() && _m.isResended()) {
            return true;
          }
          if(statesMap.containsKey(_m.getState())) {
            return visitor.visit(_m);
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
  public int countMessages(DcpConnection conn, final MessageFilter filter) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Count messages for delivery: id="+filter.getDeliveryId());
    }
    if(filter.isNoResended()) {
      loadResended(conn, filter.getDeliveryId(), 1000);
    }

    final EnumMap<MessageState, Object> statesMap = buildStatesMap(filter);

    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Process messages: id="+filter.getDeliveryId());
      }
      int _reqId = conn.getMessagesWithFields(filter, MessageField.State);

      final int[] result = new int[]{0};

      new VisitorHelperImpl(1000, _reqId, conn).visit(new Visitor<Message>() {
        @Override
        public boolean visit(Message _m) throws AdminException {
          if(filter.isNoResended() && resended.contains(_m.getId())) {
            return true;
          }
          if(statesMap.containsKey(_m.getState())) {
            result[0]++;
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
