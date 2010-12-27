package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 03.11.2010
 * Time: 14:35:50
 */
public class RestrictionException extends AdminException {

  RestrictionException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  RestrictionException(String key, String... args) {
    super(key, args);
  }
}
