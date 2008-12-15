package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.Command;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.group.datasource.Principal;

import java.util.Collection;

/**
 * User: artem
 * Date: 21.11.2008
 */
public class PrincipalListCmd extends Command {

  public String toString() {
    return "PrincipalListCmd";
  }

  public interface Receiver {
    public Collection<Principal> execute(PrincipalListCmd cmd) throws CommandExecutionException;
  }
}
