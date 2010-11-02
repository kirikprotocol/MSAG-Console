package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 01.11.2010
 * Time: 16:28:11
 */
public class DeliveryStatException extends AdminException {

  protected DeliveryStatException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  protected DeliveryStatException(String key, String... args) {
    super(key, args);
  }
}
