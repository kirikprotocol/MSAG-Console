package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Извлечение информации по DCP частями
 *
 * @author Aleksandr Khalitov
 */
abstract class DeliveryDataSource<T> {

  private final int pieceSize;

  private final DcpConnection dcpConnection;

  private final int reqId;

  DeliveryDataSource(int pieceSize, int reqId, DcpConnection dcpConnection) {
    this.pieceSize = pieceSize;
    this.dcpConnection = dcpConnection;
    this.reqId = reqId;
  }


  void visit(Visitor<T> visitor) throws AdminException {
    boolean exit;
    List<T> list = new ArrayList<T>(pieceSize);
    do {
      list.clear();
      exit = load(dcpConnection, pieceSize, reqId, list);
      for (T t : list) {
        if (!visitor.visit(t)) {
          return;
        }
      }
    } while (!exit);
  }

  protected abstract boolean load(DcpConnection connection, int pieseSize, int reqId, Collection<T> result) throws AdminException;


}
