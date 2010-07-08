package ru.novosoft.smsc.admin.reschedule;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class RescheduleException extends AdminException {
  protected RescheduleException(String key, Throwable cause) {
    super(key, cause);
  }

  protected RescheduleException(String key) {
    super(key);
  }

  protected RescheduleException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
