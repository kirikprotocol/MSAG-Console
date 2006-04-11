package ru.sibinco.scag.backend.protocol.commands.operators;

import ru.sibinco.scag.backend.daemon.Command;
import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 04.04.2006
 * Time: 13:37:09
 * To change this template use File | Settings | File Templates.
 */
public class ReloadOperators extends Command {
  public ReloadOperators() throws SibincoException {
    super("reloadOperators","command_gw.dtd");
  }
}
