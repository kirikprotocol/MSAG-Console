package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.util.Collection;
import java.util.EnumMap;
import java.util.HashSet;
import java.util.Set;

/**
 * Алгориитм извлечения сообщений из информера для случая, когда требуется достать сообщения не в
 * финальном статусе.
 *
 * @author Artem Snopkov
 */
class GetNonFinalMessagesStrategy implements GetMessagesStrategy {

  boolean[] ids = new boolean[1000];

  Set<Long> idsSet = new HashSet<Long>();

  int newMsgs = 0;
  int procMessages = 0;
  int dlvrdMsgs = 0;
  int failedMsgs = 0;
  int expMsgs = 0;

  private boolean[] extendArray(boolean[] arr, int toSize) {
    boolean[] newArr = new boolean[toSize];
    System.arraycopy(arr, 0, newArr, 0, arr.length);
    return newArr;
  }

  public void getMessages(DcpConnection conn, MessageFilter filter, int _pieceSize, final Visitor<Message> visitor) throws AdminException {

    final EnumMap<MessageState, Object> states = new EnumMap<MessageState, Object>(MessageState.class);
    if (filter.getStates() != null) {
      for (MessageState s : filter.getStates())
        states.put(s, null);
    } else {
      for (MessageState s : MessageState.values())
        states.put(s, null);
    }

    MessageFilter f = new MessageFilter(filter);

    // Сначала выгребаем все финализированные сообщения и запоминаем их идентификаторы
    f.setStates(MessageState.Delivered, MessageState.Failed, MessageState.Expired);

    int _reqId = conn.getMessages(f);
    final boolean st[] = new boolean[]{true};
    new VisitorHelperImpl(_pieceSize, _reqId, conn).visit(new Visitor<Message> () {
      public boolean visit(Message value) throws AdminException {
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

  //CHECKME me
  public int countMessages(DcpConnection conn, MessageFilter messageFilter) throws AdminException {
//    final int counter[] = new int[]{0};
//    getMessages(conn, messageFilter, 1000, new Visitor<Message>() {
//      public boolean visit(Message value) throws AdminException {
//        counter[0]++;
//        return true;
//      }
//    });
//    return counter[0];

    boolean hasNewStatus = containsStatus(messageFilter, MessageState.New);
    if (hasNewStatus) {
      MessageFilter f = new MessageFilter(messageFilter);
      f.setStates(MessageState.New);
      newMsgs = conn.countMessages(f);
    }

    MessageFilter f = new MessageFilter(messageFilter);

    f.setStates(MessageState.Delivered, MessageState.Failed, MessageState.Expired, MessageState.Process);

    int _reqId = conn.getMessages(f);

    new VisitorHelperImpl(1000, _reqId, conn).visit(new Visitor<Message> () {
      public boolean visit(Message value) throws AdminException {
        switch (value.getState()) {
          case Process:
            idsSet.add(value.getId());
            break;
          case Delivered:
            dlvrdMsgs++;
            idsSet.remove(value.getId());
            break;
          case Failed:
            failedMsgs++;
            idsSet.remove(value.getId());
            break;
          default:
            expMsgs++;
            idsSet.remove(value.getId());
            break;
        }

        return true;
      }
    });

    newMsgs -= (idsSet.size() + dlvrdMsgs + failedMsgs + expMsgs);

    int total = 0;
    if (messageFilter.getStates() != null) {
      for (MessageState s : messageFilter.getStates()) {
        switch (s) {
          case New: total += newMsgs; break;
          case Process: total += idsSet.size(); break;
          case Delivered: total += dlvrdMsgs; break;
          case Failed: total += failedMsgs; break;
          case Expired: total += expMsgs; break;
        }
      }
    } else {
      total = newMsgs + idsSet.size() + dlvrdMsgs + failedMsgs + expMsgs;
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
