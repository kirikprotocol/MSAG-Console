package ru.sibinco.lib.backend.daemon;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Command;


/**
 * Created by igork
 * Date: 03.06.2004
 * Time: 16:17:09
 */
public class CommandListServices extends Command
{
  public CommandListServices() throws SibincoException
  {
    super("list_services");
  }
}
