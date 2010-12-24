package mobi.eyeline.informer.admin.dep;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class IntegrityException extends AdminException {


  public IntegrityException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  public IntegrityException(String key, String... args) {
    super(key, args);
  }
}
