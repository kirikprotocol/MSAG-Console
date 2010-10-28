package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

import java.io.IOException;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 20.10.2010
 * Time: 15:15:34
 */
public class DeliveryStatException extends AdminException {
  
  protected DeliveryStatException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  public DeliveryStatException(String key, String... args) {
    super(key, args);
  }
}
