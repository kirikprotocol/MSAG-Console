package ru.sibinco.smppgw.backend.protocol.commands;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 24.03.2005
 * Time: 15:19:02
 * To change this template use File | Settings | File Templates.
 */
public class TraceRoute extends Command
{
  private final String applyTarget;

public TraceRoute(final String applyTarget) throws SibincoException
  {
    super("traceRoute", "file:///command_gw.dtd");
    this.applyTarget = applyTarget;
  }

  public String getApplyTarget()
  {
    return applyTarget;
  }
}
