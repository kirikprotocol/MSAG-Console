package ru.novosoft.smsc.admin.topmon;

import ru.novosoft.smsc.admin.AdminException;

/**
 * author: Aleksandr Khalitov
 */
class TopMonitorException extends AdminException{
  TopMonitorException(String key, Throwable cause) {
    super(key, cause);
  }

}
