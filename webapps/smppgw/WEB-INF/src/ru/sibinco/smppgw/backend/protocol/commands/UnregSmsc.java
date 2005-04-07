package ru.sibinco.smppgw.backend.protocol.commands;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.sme.SmscInfo;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 07.04.2005
 * Time: 15:32:14
 * To change this template use File | Settings | File Templates.
 */
public class UnregSmsc extends Command
{
  public UnregSmsc(SmscInfo smscInfo) throws SibincoException

  {
    super("unregSme", "file:///command_gw.dtd");
    createStringParam("id", smscInfo.getId());
  }
}