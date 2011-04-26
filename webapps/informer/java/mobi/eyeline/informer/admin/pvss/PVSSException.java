package mobi.eyeline.informer.admin.pvss;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class PVSSException extends AdminException{

  protected PVSSException(String key, String... args) {
    super(key, args);
  }
}
