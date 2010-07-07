package ru.novosoft.smsc.admin.service.resource_group;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ResourceGroupException extends AdminException {
  protected ResourceGroupException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ResourceGroupException(String key) {
    super(key);
  }

  protected ResourceGroupException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
