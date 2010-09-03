package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class RouteException extends AdminException {
  protected RouteException(String key, Throwable cause) {
    super(key, cause);
  }

  protected RouteException(String key) {
    super(key);
  }

  protected RouteException(String key, String causeMessage) {
    super(key, causeMessage);
  }
}
