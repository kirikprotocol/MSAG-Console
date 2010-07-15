package ru.novosoft.smsc.admin.closed_groups;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ClosedGroupException extends AdminException {

  protected ClosedGroupException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ClosedGroupException(String key) {
    super(key);
  }

  protected ClosedGroupException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
