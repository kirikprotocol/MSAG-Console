package ru.sibinco.scag.backend.protocol.commands;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.sme.GwSme;


/**
 * Created by igork Date: 20.08.2004 Time: 17:13:35
 */
public class UpdateSmeInfo extends SmeCommand
{
  public UpdateSmeInfo(final GwSme gwSme)
      throws SibincoException
  {
    super("updateSmeInfo", "file:///command_gw.dtd");
    fillParams(gwSme);
  }

}
