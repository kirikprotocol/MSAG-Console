package mobi.eyeline.informer.admin.protocol;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
class ProtocolException extends AdminException {
  ProtocolException(String key, Throwable cause) {
    super(key, cause);
  }

  ProtocolException(String key) {
    super(key);
  }
}
