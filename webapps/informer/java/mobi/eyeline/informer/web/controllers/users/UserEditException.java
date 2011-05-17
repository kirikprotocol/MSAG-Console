package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.util.LocalizedException;

/**
 * User: artem
 * Date: 17.05.11
 */
public class UserEditException extends LocalizedException  {
  protected UserEditException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }
}
