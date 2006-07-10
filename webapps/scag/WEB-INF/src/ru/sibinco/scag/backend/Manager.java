package ru.sibinco.scag.backend;

import ru.sibinco.lib.SibincoException;


/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 30.06.2006
 * Time: 14:00:22
 * To change this template use File | Settings | File Templates.
 */
public abstract class Manager {
  public abstract void store() throws SibincoException;
  public void parse() throws Throwable {
  }
}
