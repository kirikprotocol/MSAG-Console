package mobi.eyeline.informer.admin.delivery;

import java.util.List;

/**
 * Извлечение информации по DCP частями
 * @author Aleksandr Khalitov
 */
public abstract class DeliveryDataSource<T> {

  protected final int pieceSize;

  protected DcpConnection dcpConnection;

  protected int reqId;

  protected DeliveryDataSource(int pieceSize, int reqId, DcpConnection dcpConnection) {
    this.pieceSize = pieceSize;
    this.dcpConnection = dcpConnection;
    this.reqId = reqId;
  }

  public int getPieceSize() {
    return pieceSize;
  }


  public void visit() {




  }




  protected List<T> load() {
    return null;
  }
}
