package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.util.List;

/**
 * Извлечение информации по DCP частями
 * @author Aleksandr Khalitov
 */
public abstract class DeliveryVisitor<T> {

  private final int pieceSize;

  protected DcpConnection dcpConnection;

  protected int reqId;

  protected DeliveryVisitor(int pieceSize, int reqId, DcpConnection dcpConnection) {
    this.pieceSize = pieceSize;
    this.dcpConnection = dcpConnection;
    this.reqId = reqId;
  }

  public int getPieceSize() {
    return pieceSize;
  }


  public void visit() throws AdminException{
//    LinkedList<>




  }




  protected abstract List<T> load(int pieseSize) throws AdminException;
}
