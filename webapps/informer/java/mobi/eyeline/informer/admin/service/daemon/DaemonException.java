package mobi.eyeline.informer.admin.service.daemon;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
class DaemonException extends AdminException {
  protected DaemonException(String key, Throwable cause) {
    super(key, cause);
  }

  DaemonException(String key, String... args) {
    super(key, args);
  }
}
