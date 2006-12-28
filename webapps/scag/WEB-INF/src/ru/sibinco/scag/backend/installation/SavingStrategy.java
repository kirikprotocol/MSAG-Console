package ru.sibinco.scag.backend.installation;

import ru.sibinco.lib.SibincoException;

import java.io.File;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 26.12.2006
 * Time: 18:06:30
 * To change this template use File | Settings | File Templates.
 */
public interface SavingStrategy {
  public void storeToMirror(File mirrorFile) throws SibincoException;
}
