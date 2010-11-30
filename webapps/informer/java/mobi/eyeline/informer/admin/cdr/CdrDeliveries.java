package mobi.eyeline.informer.admin.cdr;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;

/**
 * @author Aleksandr Khalitov
 */
public interface CdrDeliveries {

  public Delivery getDelviery(String user, int deliveryId) throws AdminException;
  
}
