package mobi.eyeline.informer.admin;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 14.10.2010
 * Time: 15:54:35
 */
public class IntegrityException extends AdminException {

  IntegrityException(String key, String... args) {
    super(key, args);
  }
}
