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

  private int timeoutSec;
  private int messagesCountPiece;

  GetFinalizedMessagesStrategy(int timeoutSec, int messagesCountPiece) {
    this.timeoutSec = timeoutSec;
    this.messagesCountPiece = messagesCountPiece;
  }

  public void getMessages(DcpConnection conn, MessageFilter filter, int _pieceSize, Visitor<Message> visitor) throws AdminException {

    int _reqId = conn.getMessages(filter);

    new VisitorHelper<Message>(_pieceSize, _reqId, conn) {
      protected boolean load(DcpConnection connection, int pieceSize, int reqId, Collection<Message> result) throws AdminException {
        return connection.getNextMessages(reqId, pieceSize, timeoutSec, result);
      }
    }.visit(visitor);

  }

  public int countMessages(DcpConnection conn, MessageFilter messageFilter) throws AdminException {
    int reqId = conn.countMessages(messageFilter);
    int[] tmpRes = new int[]{0};
    int res = 0;
    boolean more;
    do {
      more= conn.getNextMessagesCount(reqId, messagesCountPiece, timeoutSec, tmpRes);
      res+=tmpRes[0];
    }while (more);
    return res;
  }

}
