package mobi.eyeline.informer.admin.service;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ServiceManagerException extends AdminException {
  
  protected ServiceManagerException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ServiceManagerException(String key, String ... args) {
    super(key, args);
  }

  
}
