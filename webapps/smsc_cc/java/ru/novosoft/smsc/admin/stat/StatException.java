package ru.novosoft.smsc.admin.stat;

import ru.novosoft.smsc.admin.AdminException;

/**
 * author: Aleksandr Khalitov
 */
class StatException extends AdminException{

  StatException(String key, Throwable cause) {
    super(key, cause);
  }

  StatException(String key) {
    super(key);
  }

  StatException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
