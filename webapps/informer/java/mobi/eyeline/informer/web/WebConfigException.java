package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class WebConfigException extends AdminException {
  protected WebConfigException(String key, Throwable cause) {
    super(key, cause);
  }

  protected WebConfigException(String key) {
    super(key);
  }

  protected WebConfigException(String key, String ... args) {
    super(key, args);
  }
}
