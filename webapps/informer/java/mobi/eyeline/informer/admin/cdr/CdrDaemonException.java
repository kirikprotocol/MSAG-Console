package mobi.eyeline.informer.admin.cdr;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class CdrDaemonException extends AdminException{
  CdrDaemonException(String key, String... args) {
    super(key, args);
  }
}
