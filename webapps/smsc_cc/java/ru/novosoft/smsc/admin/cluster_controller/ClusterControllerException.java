package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ClusterControllerException extends AdminException {
  protected ClusterControllerException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ClusterControllerException(String key) {
    super(key);
  }

  protected ClusterControllerException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
