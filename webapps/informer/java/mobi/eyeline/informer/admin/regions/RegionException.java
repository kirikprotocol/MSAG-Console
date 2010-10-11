package mobi.eyeline.informer.admin.regions;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public class RegionException extends AdminException{
  
  public RegionException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }

  public RegionException(String key, String... args) {
    super(key, args);
  }
}
