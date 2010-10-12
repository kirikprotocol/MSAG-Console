package mobi.eyeline.informer.admin.protocol;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ProtocolException extends AdminException {
  protected ProtocolException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ProtocolException(String key) {
    super(key);
  }
}
