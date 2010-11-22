package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class InfosmeException extends AdminException {

  public InfosmeException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  public InfosmeException(String key, Throwable cause) {
    super(key, cause);
  }

  public InfosmeException(String key, String... args) {
    super(key, args);
  }
}
