package mobi.eyeline.informer.admin.service.resource_group;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ResourceGroupException extends AdminException {
  protected ResourceGroupException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ResourceGroupException(String key, String ... args) {
    super(key, args);
  }
}
