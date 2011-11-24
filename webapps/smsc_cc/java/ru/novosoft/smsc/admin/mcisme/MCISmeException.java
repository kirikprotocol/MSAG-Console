package ru.novosoft.smsc.admin.mcisme;

import ru.novosoft.smsc.admin.AdminException;

/**
 * author: Aleksandr Khalitov
 */
class MCISmeException extends AdminException {

  private Status status = Status.UNKNOWN;

  protected MCISmeException(String key, Throwable cause) {
    super(key, cause);
  }

  protected MCISmeException(String key, Throwable cause, Status status) {
    super(key, cause);
    this.status = status;
  }

  protected MCISmeException(String key) {
    super(key);
  }

  protected MCISmeException(String key, Status status) {
    super(key);
    this.status = status;
  }

  MCISmeException(String key, String causeMessage) {
    super(key, causeMessage);
  }

  MCISmeException(String key, String causeMessage, Status status) {
    super(key, causeMessage);
    this.status = status;
  }

  public Status getStatus() {
    return status;
  }

  enum Status {
    SUBSCRIBER_NOT_FOUND, SYSTEM_ERROR, UNKNOWN
  }
}
