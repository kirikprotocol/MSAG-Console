package mobi.eyeline.informer.admin.monitoring;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
class MonitoringException extends AdminException{
  MonitoringException(String key, String... args) {
    super(key, args);
  }
}
