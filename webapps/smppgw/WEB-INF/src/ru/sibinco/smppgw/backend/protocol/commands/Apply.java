package ru.sibinco.smppgw.backend.protocol.commands;

import ru.sibinco.lib.backend.protocol.Command;


/**
 * Created by igork
 * Date: 26.05.2004
 * Time: 16:35:20
 */
public class Apply extends Command
{
  private final String applyTarget;

  public Apply(String applyTarget)
  {
    super("apply");
    this.applyTarget = applyTarget;
  }

  public String getApplyTarget()
  {
    return applyTarget;
  }
}
