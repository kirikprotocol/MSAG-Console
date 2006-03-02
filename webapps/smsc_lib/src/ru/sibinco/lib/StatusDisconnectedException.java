package ru.sibinco.lib;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 27.02.2006
 * Time: 18:12:42
 * To change this template use File | Settings | File Templates.
 */
public class StatusDisconnectedException extends SibincoException{
  public StatusDisconnectedException(String host, int port) {
    super("Could't connect to " + host + ':' + port);
  }
}
