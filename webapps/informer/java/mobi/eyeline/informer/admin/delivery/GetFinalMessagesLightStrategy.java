package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import org.apache.log4j.Logger;

import java.util.Collection;
import java.util.Date;
import java.util.HashSet;
import java.util.Set;

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

  private void loadResended(DcpConnection conn, int deliveryId, int _pieceSize) throws AdminException {
    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Load resended: id="+deliveryId);
      }

      Delivery d = conn.getDelivery(deliveryId);

      MessageFilter filter = new MessageFilter(d.getId(), d.getCreateDate(), d.getEndDate() != null ? d.getEndDate() : new Date(System.currentTimeMillis()+(24*60*60*1000)));
      filter.setStates(MessageState.New);

      int _reqId = conn.getMessagesWithFields(filter, MessageField.UserData, MessageField.State);
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

    visitMessages(conn, filter, new Visitor<Message>() {
      @Override
      public boolean visit(Message _m) throws AdminException {
        if(resended.contains(_m.getId())) {
          _m.setResended(true);
        }
        if(filter.isNoResended() && _m.isResended()) {
          return true;
        }
        return visitor.visit(_m);
      }
    }, MessageField.values());
  }

  @Override
  public int countMessages(DcpConnection conn, final MessageFilter filter) throws AdminException {
    if(logger.isDebugEnabled()) {
      logger.debug("Count messages for delivery: id="+filter.getDeliveryId());
    }
    if(filter.isNoResended()) {
      loadResended(conn, filter.getDeliveryId(), 1000);
    }

    final int[] result = new int[]{0};

    visitMessages(conn, filter, new Visitor<Message>() {
      @Override
      public boolean visit(Message _m) throws AdminException {
        if(filter.isNoResended() && resended.contains(_m.getId())) {
          return true;
        }
        result[0]++;
        return true;
      }
    }, new MessageField[0]);
    return result[0];
  }


  private void visitMessages(DcpConnection conn, final MessageFilter filter, Visitor<Message> visitor, MessageField ... fields) throws AdminException {
    long now = System.currentTimeMillis();
    try{
      if(logger.isDebugEnabled()) {
        logger.debug("Visit messages: id="+filter.getDeliveryId());
      }
      int _reqId = conn.getMessagesWithFields(filter, fields);
      new VisitorHelperImpl(1000, _reqId, conn).visit(visitor);
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
}
