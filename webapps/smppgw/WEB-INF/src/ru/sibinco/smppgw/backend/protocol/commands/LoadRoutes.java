package ru.sibinco.smppgw.backend.protocol.commands;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 24.03.2005
 * Time: 15:02:49
 * To change this template use File | Settings | File Templates.
 */
public class LoadRoutes extends Command
{
  private final String applyTarget;

public LoadRoutes(final String applyTarget) throws SibincoException
  {
    super("loadRoutes", "file:///command_gw.dtd");
    this.applyTarget = applyTarget;
    createStringParam("subj", this.applyTarget);
  }

  public String getApplyTarget()
  {
    return applyTarget;
  }
}