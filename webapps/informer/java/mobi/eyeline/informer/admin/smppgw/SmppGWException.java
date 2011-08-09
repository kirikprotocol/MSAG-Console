package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;

/**
 * author: Aleksandr Khalitov
 */
class SmppGWException extends AdminException {

  SmppGWException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  SmppGWException(String key, String... args) {
    super(key, args);
  }
}
