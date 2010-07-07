package ru.novosoft.smsc.admin.alias;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class AliasManagerException extends AdminException {
  protected AliasManagerException(String key, Throwable cause) {
    super(key, cause);
  }
}
