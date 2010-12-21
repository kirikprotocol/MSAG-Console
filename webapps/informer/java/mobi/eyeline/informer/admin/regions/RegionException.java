package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class RegionException extends AdminException {

  RegionException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  RegionException(String key, String... args) {
    super(key, args);
  }
}
