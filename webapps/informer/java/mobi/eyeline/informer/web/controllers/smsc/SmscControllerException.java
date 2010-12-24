package mobi.eyeline.informer.web.controllers.smsc;

import mobi.eyeline.informer.util.LocalizedException;

/**
 * @author Artem Snopkov
 */
public class SmscControllerException extends LocalizedException {
  protected SmscControllerException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  protected SmscControllerException(String key, String... args) {
    super(key, args);
  }
}
