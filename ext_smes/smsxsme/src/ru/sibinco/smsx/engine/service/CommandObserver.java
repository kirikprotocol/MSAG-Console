package ru.sibinco.smsx.engine.service;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public interface CommandObserver {
  public void update(AsyncCommand command);
}
