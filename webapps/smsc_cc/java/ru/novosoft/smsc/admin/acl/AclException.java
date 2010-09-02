package ru.novosoft.smsc.admin.acl;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class AclException extends AdminException {
  protected AclException(String key, Throwable cause) {
    super(key, cause);
  }

  protected AclException(String key) {
    super(key);
  }

  protected AclException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
