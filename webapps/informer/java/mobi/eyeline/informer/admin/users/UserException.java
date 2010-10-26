package mobi.eyeline.informer.admin.users;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 15.10.2010
 * Time: 12:41:32
 */
public class UserException extends AdminException{

  public UserException(String key, String... args) {
    super(key, args);
  }

  protected UserException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }
}
