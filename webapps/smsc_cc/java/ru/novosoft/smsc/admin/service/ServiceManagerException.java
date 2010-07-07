package ru.novosoft.smsc.admin.service;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ServiceManagerException extends AdminException {
  protected ServiceManagerException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ServiceManagerException(String key) {
    super(key);
  }

  protected ServiceManagerException(String key, String causeMessage) {
    super(key, causeMessage);
  }

  
}
