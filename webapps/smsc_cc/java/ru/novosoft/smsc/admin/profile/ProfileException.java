package ru.novosoft.smsc.admin.profile;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ProfileException extends AdminException {
  
  protected ProfileException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ProfileException(String key) {
    super(key);
  }

  protected ProfileException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
