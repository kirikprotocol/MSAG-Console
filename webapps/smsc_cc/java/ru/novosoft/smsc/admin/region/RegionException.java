package ru.novosoft.smsc.admin.region;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class RegionException extends AdminException {
  protected RegionException(String key, Throwable cause) {
    super(key, cause);
  }

  protected RegionException(String key) {
    super(key);
  }

  protected RegionException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
