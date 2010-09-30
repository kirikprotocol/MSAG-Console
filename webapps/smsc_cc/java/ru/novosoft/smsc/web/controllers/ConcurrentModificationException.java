package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.AdminException;

/**
 * @author Artem Snopkov
 */
public class ConcurrentModificationException extends AdminException {
  protected ConcurrentModificationException(String user, long lastUpdateTime) {
    super("text", user);
  }
}
