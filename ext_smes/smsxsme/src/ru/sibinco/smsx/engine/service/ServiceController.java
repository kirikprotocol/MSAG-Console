package ru.sibinco.smsx.engine.service;

/**
 * User: artem
 * Date: 05.07.2007
 */

public interface ServiceController {
  public void startService();
  public void stopService();
  public Object getMBean(String domain);
}
