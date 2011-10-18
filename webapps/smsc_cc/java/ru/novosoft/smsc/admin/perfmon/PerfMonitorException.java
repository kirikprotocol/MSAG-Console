package ru.novosoft.smsc.admin.perfmon;

import ru.novosoft.smsc.admin.AdminException;

/**
 * author: Aleksandr Khalitov
 */
class PerfMonitorException extends AdminException {
  protected PerfMonitorException(String key, Throwable cause) {
    super(key, cause);
  }

  protected PerfMonitorException(String key) {
    super(key);
  }

  protected PerfMonitorException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
