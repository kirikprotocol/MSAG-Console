package mobi.eyeline.informer.web.controllers.regions;

import mobi.eyeline.informer.util.LocalizedException;

/**
 * User: artem
 * Date: 13.01.11
 */
public class RegionsControllerException extends LocalizedException {

  RegionsControllerException(String key, String... args) {
    super(key, args);
  }
}
