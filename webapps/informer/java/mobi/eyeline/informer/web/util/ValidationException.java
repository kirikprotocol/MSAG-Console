package mobi.eyeline.informer.web.util;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ValidationException extends AdminException {

  protected ValidationException(String bundleName, String key,  String ... args) {
    super(bundleName, key, args);
  }

}
