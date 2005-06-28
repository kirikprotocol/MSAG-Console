package ru.sibinco.scag.backend.protocol.commands;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Command;


/**
 * Created by igork Date: 01.09.2004 Time: 14:38:41
 */
public class DeleteSme extends Command
{
  public DeleteSme(final String smeId) throws SibincoException
  {
    super("deleteSme", "file:///command_gw.dtd");
    createStringParam("id", smeId);
  }
}
