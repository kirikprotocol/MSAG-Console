package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class InformerException extends AdminException {

  InformerException(String key, String... args) {
    super(key, args);
  }
}
