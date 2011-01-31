package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 18.11.2010
 * Time: 13:27:30
 */
public class ContentProviderException extends AdminException {
  public ContentProviderException(String key, String... args) {
    super(key, args);
  }

  public ContentProviderException(String key, Throwable cause, String... args) {
    super(key, cause, args);
  }
}
