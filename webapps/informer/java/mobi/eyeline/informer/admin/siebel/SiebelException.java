package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelException extends AdminException {

  SiebelException(String key, String... args) {
    super(key, args);
  }
}
