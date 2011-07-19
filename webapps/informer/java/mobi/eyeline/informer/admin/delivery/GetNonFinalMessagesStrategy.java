package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.util.*;

/**
 * Алгориитм извлечения сообщений из информера для случая, когда требуется достать сообщения не в
 * финальном статусе.
 *
 * @author Artem Snopkov
 */
class GetNonFinalMessagesStrategy implements GetMessagesStrategy {

  boolean[] ids = new boolean[1000];

  Set<Long> processSet = new HashSet<Long>();
  Set<Long> retrySet = new HashSet<Long>();
  Set<Long> sentSet = new HashSet<Long>();

  int newMsgs = 0;
  int dlvrdMsgs = 0;
  int failedMsgs = 0;
  int expMsgs = 0;

  private final String resendProperty;

  GetNonFinalMessagesStrategy(String resendProperty) {
    this.resendProperty = resendProperty;
  }

  private boolean[] extendArray(boolean[] arr, int toSize) {
    boolean[] newArr = new boolean[toSize];
    System.arraycopy(arr, 0, newArr, 0, arr.length);
    return newArr;
  }

  private Set<Long> getResended(DcpConnection conn, int deliveryId, int _pieceSize) throws AdminException {
    Delivery d = conn.getDelivery(deliveryId);

    final Set<Long> resendedSet = new HashSet<Long>();

    MessageFilter _filter = new MessageFilter(d.getId(), d.getCreateDate(), d.getEndDate() != null ? d.getEndDate() : new Date(System.currentTimeMillis()+(24*60*60*1000)));
    int _reqId = conn.getMessages(_filter);
    new VisitorHelperImpl(_pieceSize, _reqId, conn).visit(new Visitor<Message>() {
      @Override
      public boolean visit(Message _m) throws AdminException {
        String _p =_m.getProperty(resendProperty);
        if(_p != null) {
          resendedSet.add(Long.parseLong(_p));
        }
        return true;
      }
    });
    return resendedSet;
  }


  public void getMessages(DcpConnection conn, MessageFilter filter, int _pieceSize, final Visitor<Message> visitor) throws AdminException {

    final Set<Long> resended = getResended(conn, filter.getDeliveryId(), _pieceSize);

    final EnumMap<MessageState, Object> states = new EnumMap<MessageState, Object>(MessageState.class);
    if (filter.getStates() != null) {
      for (MessageState s : filter.getStates())
        states.put(s, null);
    } else {
      for (MessageState s : MessageState.values())
        states.put(s, null);
    }

    final MessageFilter f = new MessageFilter(filter);

    // Сначала выгребаем все финализированные сообщения и запоминаем их идентификаторы
    f.setStates(MessageState.Delivered, MessageState.Failed, MessageState.Expired);

    int _reqId = conn.getMessages(f);
    final boolean st[] = new boolean[]{true};
    new VisitorHelperImpl(_pieceSize, _reqId, conn).visit(new Visitor<Message> () {
      public boolean visit(Message value) throws AdminException {
        if(resended.contains(value.getId())) {
          if(f.isNoResended()) {
            return true;
          }
          value.setResended(true);
        }
        int id = (int)value.getId().longValue();
        if (ids.length <= id)
          ids = extendArray(ids, id + 1000);
        ids[id] = true;

        return !states.containsKey(value.getState()) || (st[0] = visitor.visit(value));
      }
    });

    if (!st[0])
      return;

    // Далее выгребаем все сообщения в статусе Proccess, чьи идентификаторы отсутствуют в ids, и запоминаем их идентификаторы
    f.setStates(MessageState.Process);
    final boolean containsProcState = states.containsKey(MessageState.Process);

    _reqId = conn.getMessages(f);
    new VisitorHelperImpl(_pieceSize, _reqId, conn).visit(new Visitor<Message> () {

      public boolean visit(Message value) throws AdminException {
        if(resended.contains(value.getId())) {
          if(f.isNoResended()) {
            return true;
          }
          value.setResended(true);
        }
        int id = (int)value.getId().longValue();

        if (ids.length > id) {
          if (ids[id])
            return true;
        } else {
          ids = extendArray(ids, id + 1000);
        }
        ids[id] = true;

        if (containsProcState) {
          st[0] = visitor.visit(value);
          return st[0];
        }

        return true;
      }
    });

    if (!st[0] || !states.containsKey(MessageState.New))
      return;

    Delivery delivery = conn.getDelivery(filter.getDeliveryId());
    String singleText = null;
    if (delivery.getProperty("singleText") != null)
      singleText = conn.getDeliveryGlossary(filter.getDeliveryId())[0];
    final String fSingleText = singleText;

    // Наконец, выгребаем все сообщения в статусе New, чьи идентификаторы отсутствуют в ids
    f.setStates(MessageState.New);

    _reqId = conn.getMessages(f);
    new VisitorHelperImpl(_pieceSize, _reqId, conn).visit(new Visitor<Message> () {
      public boolean visit(Message value) throws AdminException {
        if(resended.contains(value.getId())) {
          if(f.isNoResended()) {
            return true;
          }
          value.setResended(true);
        }
        int id = (int)value.getId().longValue();

        if (ids.length > id && ids[id])
          return true;

        if (fSingleText != null)
          value.setText(fSingleText);
        return  visitor.visit(value);
      }
    });
  }

  private static boolean containsStatus(MessageFilter messageFilter, MessageState status) {
    if (messageFilter.getStates() == null)
      return true;
    else {
      for (MessageState s: messageFilter.getStates())
        if (s == status) {
          return true;
        }
    }
    return false;
  }

  public int countMessages(DcpConnection conn, final MessageFilter messageFilter) throws AdminException {
//    final int counter[] = new int[]{0};
//    getMessages(conn, messageFilter, 1000, new Visitor<Message>() {
//      public boolean visit(Message value) throws AdminException {
//        counter[0]++;
//        return true;
//      }
//    });
//    return counter[0];

    final Set<Long> resended = getResended(conn, messageFilter.getDeliveryId(), 1000);

    boolean hasNewStatus = containsStatus(messageFilter, MessageState.New);
    if (hasNewStatus) {
      MessageFilter f = new MessageFilter(messageFilter);
      f.setStates(MessageState.New);

      int reqId = conn.getMessages(f);
      final int[] tmpRes = new int[]{0};
      new VisitorHelperImpl(1000, reqId, conn).visit(new Visitor<Message> () {
        public boolean visit(Message value) throws AdminException {
          if (!messageFilter.isNoResended() || !resended.contains(value.getId())) {
            tmpRes[0]++;
          }
          return true;
        }
      });
      newMsgs = tmpRes[0];
    }

    MessageFilter f = new MessageFilter(messageFilter);

    f.setStates(MessageState.Delivered, MessageState.Failed, MessageState.Expired, MessageState.Process, MessageState.Sent, MessageState.Retry);

    int _reqId = conn.getMessages(f);

    new VisitorHelperImpl(1000, _reqId, conn).visit(new Visitor<Message> () {
      public boolean visit(Message value) throws AdminException {
        if(messageFilter.isNoResended()) {
          if(resended.contains(value.getId())) {
            return true;
          }
        }
        switch (value.getState()) {
          case Process:
            processSet.add(value.getId());
            break;
          case Sent:
            processSet.remove(value.getId());
            sentSet.add(value.getId());
            break;
          case Retry:
            processSet.remove(value.getId());
            sentSet.remove(value.getId());
            retrySet.add(value.getId());
            break;
          case Delivered:
            dlvrdMsgs++;
            processSet.remove(value.getId());
            sentSet.remove(value.getId());
            retrySet.remove(value.getId());
            break;
          case Failed:
            failedMsgs++;
            processSet.remove(value.getId());
            sentSet.remove(value.getId());
            retrySet.remove(value.getId());
            break;
          default:
            expMsgs++;
            processSet.remove(value.getId());
            sentSet.remove(value.getId());
            retrySet.remove(value.getId());
            break;
        }
        return true;
      }
    });

    int countResended = resended.size();
    resended.clear();

    newMsgs -= (processSet.size() + sentSet.size() + retrySet.size() + dlvrdMsgs + failedMsgs + expMsgs);

    int total = 0;
    if (messageFilter.getStates() != null) {
      for (MessageState s : messageFilter.getStates()) {
        switch (s) {
          case New: total += newMsgs; break;
          case Process: total += processSet.size(); break;
          case Retry: total += retrySet.size(); break;
          case Sent: total += sentSet.size(); break;
          case Delivered: total += dlvrdMsgs; break;
          case Failed: total += failedMsgs; break;
          case Expired: total += expMsgs; break;
        }
      }
    } else {
      total = newMsgs + processSet.size() + retrySet.size() + sentSet.size() + dlvrdMsgs + failedMsgs + expMsgs;
    }


    return total;
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
