package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.util.LocalizedException;

/**
 * @author Artem Snopkov
 */
public class DeliveryControllerException extends LocalizedException {

  protected DeliveryControllerException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  protected DeliveryControllerException(String key, String... args) {
    super(key, args);
  }
}
