package ru.novosoft.smsc.admin.category;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class CategoryException extends AdminException {
  protected CategoryException(String key, Throwable cause) {
    super(key, cause);
  }

  protected CategoryException(String key) {
    super(key);
  }

  protected CategoryException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
