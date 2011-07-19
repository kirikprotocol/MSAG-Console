package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.util.*;

/**
 *
 * Алгоритм извлечения сообщений из Информера для случая, когда запрашиваются только сообщения
 * в финальном статусе.
 *
 * @author Artem Snopkov
 */
class GetFinalizedMessagesStrategy  implements GetMessagesStrategy {

  private int messagesCountPiece;

  private final String resendProperty;

  GetFinalizedMessagesStrategy(int messagesCountPiece, String resendProperty) {
    this.messagesCountPiece = messagesCountPiece;
    this.resendProperty = resendProperty;
  }

  private Set<Long> getResended(DcpConnection conn, int deliveryId, int _pieceSize) throws AdminException {
    Delivery d = conn.getDelivery(deliveryId);

    Set<Long> resendedSet = new HashSet<Long>();

    MessageFilter _filter = new MessageFilter(d.getId(), d.getCreateDate(), d.getEndDate() != null ? d.getEndDate() : new Date(System.currentTimeMillis()+(24*60*60*1000)));
    int _reqId = conn.getMessages(_filter);
    List<Message> _ms = new ArrayList<Message>(_pieceSize);
    boolean load = true;
    while(load){
      load =  conn.getNextMessages(_reqId, _pieceSize, _ms);
      for(Message _m : _ms) {
        String _p =_m.getProperty(resendProperty);
        if(_p != null) {
          resendedSet.add(Long.parseLong(_p));
        }
      }
      _ms.clear();
    }
    return resendedSet;
  }

  private int countResended(DcpConnection conn, MessageFilter _filter, int _pieceSize) throws AdminException {
    Set<Long> resendedSet = new HashSet<Long>();
    int _reqId = conn.getMessages(_filter);
    List<Message> _ms = new ArrayList<Message>(_pieceSize);
    boolean load = true;
    while(load){
      load =  conn.getNextMessages(_reqId, _pieceSize, _ms);
      for(Message _m : _ms) {
        String _p =_m.getProperty(resendProperty);
        if(_p != null) {
          resendedSet.add(Long.parseLong(_p));
        }
      }
      _ms.clear();
    }
    return resendedSet.size();
  }


  public void getMessages(DcpConnection conn, final MessageFilter filter, final int _pieceSize, Visitor<Message> visitor) throws AdminException {

    final Set<Long> resendedSet = getResended(conn, filter.getDeliveryId(), _pieceSize);

    int _reqId = conn.getMessages(filter);

    new VisitorHelper<Message>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<Message> result) throws AdminException {
        Collection<Message> preresult = new ArrayList<Message>(pieceSize);
        boolean hasNext = connection.getNextMessages(reqId, pieceSize, preresult);
        Iterator<Message> i = preresult.iterator();
        while(i.hasNext()) {
          Message _m = i.next();
          if(resendedSet.contains(_m.getId())) {
            if(filter.isNoResended()) {
              i.remove();
            }
            _m.setResended(true);
          }

        }
        result.addAll(preresult);
        return hasNext;
      }
    }.visit(visitor);

  }

  public int countMessages(DcpConnection conn, MessageFilter messageFilter) throws AdminException {
    int res = 0;
    int reqId = conn.countMessages(messageFilter);
    int[] tmpRes = new int[]{0};
    boolean more;
    do {
      more= conn.getNextMessagesCount(reqId, messagesCountPiece, tmpRes);
      res+=tmpRes[0];
    }while (more);

    if(messageFilter.isNoResended()) {
      res-=countResended(conn, messageFilter, 1000);
    }
    return res;
  }

}
