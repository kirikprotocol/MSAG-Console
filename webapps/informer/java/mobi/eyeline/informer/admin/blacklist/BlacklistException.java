package mobi.eyeline.informer.admin.blacklist;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class BlacklistException extends AdminException{

  BlacklistException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  BlacklistException(String key, String... args) {
    super(key, args);
  }
}
