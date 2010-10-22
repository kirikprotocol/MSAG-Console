package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class DcpConverterException extends AdminException {

  DcpConverterException(String key, String... args) {
    super(key, args);
  }

}
