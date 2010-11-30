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

  private Set<Long> ids = new HashSet<Long>();

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
        ids.add(value.getId());
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
        if (ids.contains(value.getId()))
          return true;

        ids.add(value.getId());

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
        if (ids.contains(value.getId()))
          return true;
        if (fSingleText != null)
          value.setText(fSingleText);
        return  visitor.visit(value);
      }
    });

  }

  public int countMessages(DcpConnection conn, MessageFilter messageFilter) throws AdminException {
    final int counter[] = new int[]{0};
    getMessages(conn, messageFilter, 1000, new Visitor<Message>() {
      public boolean visit(Message value) throws AdminException {
        counter[0]++;
        return true;
      }
    });
    return counter[0];
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
