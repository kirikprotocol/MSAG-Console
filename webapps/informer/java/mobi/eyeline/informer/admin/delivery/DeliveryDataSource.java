package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Извлечение информации по DCP частями
 * @author Aleksandr Khalitov
 */
public abstract class DeliveryDataSource<T> {

  private final int pieceSize;

  protected DcpConnection dcpConnection;

  private final int reqId;

  protected DeliveryDataSource(int pieceSize, int reqId, DcpConnection dcpConnection) {
    this.pieceSize = pieceSize;
    this.dcpConnection = dcpConnection;
    this.reqId = reqId;
  }


  public void visit(Visitor<T> visitor) throws AdminException{
    boolean exit;
    List<T> list = new ArrayList<T>(pieceSize);
    do{
      list.clear();
      exit =  load(pieceSize, reqId, list);
      for(T t : list) {
        if(!visitor.visit(t)) {
          exit = true;
          break;
        }
      }
    }while(exit);
  }

  protected abstract boolean load(int pieseSize, int reqId, Collection<T> result) throws AdminException;


  /**
   * Визитер
   * @param <T> тип извлекаемых данных
   */
  public static interface Visitor<T> {

    public boolean visit(T value) throws AdminException;

  }
}
