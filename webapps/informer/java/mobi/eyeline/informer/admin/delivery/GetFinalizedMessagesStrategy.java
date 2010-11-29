package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.util.Collection;

/**
 *
 * Алгоритм извлечения сообщений из Информера для случая, когда запрашиваются только сообщения
 * в финальном статусе.
 *
 * @author Artem Snopkov
 */
class GetFinalizedMessagesStrategy  implements GetMessagesStrategy {

  public void getMessages(DcpConnection conn, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {

    int _reqId = conn.getMessages(filter);

    new VisitorHelper<Message>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<Message> result) throws AdminException {
        return connection.getNextMessages(reqId, pieceSize, result);
      }
    }.visit(visitor);

  }

  public int countMessages(DcpConnection conn, MessageFilter messageFilter) throws AdminException {
    return conn.countMessages(messageFilter);
  }

}
