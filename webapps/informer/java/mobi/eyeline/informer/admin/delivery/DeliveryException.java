package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryException extends AdminException {

  public DeliveryException(String key, Throwable cause) {
    super(key, cause);
  }

  public DeliveryException(String key, String... args) {
    super(key, args);
  }
}