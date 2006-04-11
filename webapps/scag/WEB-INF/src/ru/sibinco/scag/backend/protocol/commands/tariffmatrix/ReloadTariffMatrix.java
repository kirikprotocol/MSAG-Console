package ru.sibinco.scag.backend.protocol.commands.tariffmatrix;

import ru.sibinco.scag.backend.daemon.Command;
import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 04.04.2006
 * Time: 13:40:06
 * To change this template use File | Settings | File Templates.
 */
public class ReloadTariffMatrix extends Command {
  public ReloadTariffMatrix() throws SibincoException {
    super("reloadTariffMatrix","command_gw.dtd");
  }
}
