package ru.sibinco.smsx.engine.service.group.commands;

import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsc.utils.admin.dl.DistributionList;

import java.util.List;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupListCmd extends GroupCommand {
  public interface Receiver {
    public List<DistributionList> execute(GroupListCmd cmd) throws CommandExecutionException;
  }
}
