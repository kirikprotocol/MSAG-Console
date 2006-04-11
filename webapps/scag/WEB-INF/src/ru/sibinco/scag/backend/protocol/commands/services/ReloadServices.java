package ru.sibinco.scag.backend.protocol.commands.services;

import ru.sibinco.scag.backend.daemon.Command;
import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 04.04.2006
 * Time: 13:39:00
 * To change this template use File | Settings | File Templates.
 */
public class ReloadServices extends Command {
  public ReloadServices() throws SibincoException {
    super("reloadServices","command_gw.dtd");
  }
}
