package ru.sibinco.smppgw.backend.protocol.commands;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.sme.GwSme;


/**
 * Created by igork Date: 01.09.2004 Time: 14:48:46
 */
public class AddSme extends SmeCommand
{
  public AddSme(final GwSme gwSme) throws SibincoException
  {
    super("addSme", "file:///command_gw.dtd");
    fillParams(gwSme);
  }
}
