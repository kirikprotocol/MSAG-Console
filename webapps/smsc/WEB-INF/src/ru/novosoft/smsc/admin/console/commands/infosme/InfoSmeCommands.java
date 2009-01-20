package ru.novosoft.smsc.admin.console.commands.infosme;

import ru.novosoft.smsc.admin.console.CommandContext;

/**
 * User: artem
 * Date: Aug 6, 2007
 */

public interface InfoSmeCommands {
  public void importFile(CommandContext ctx, String file);

  public void createTask(CommandContext ctx, Distribution distibution);

  public void getStatus(CommandContext ctx, String taskId);

  public void resendMessage(CommandContext ctx, String msisdn, String taskId, String text);

  public void removeTask(CommandContext ctx, String taskId);
  
  public void alterTask(CommandContext ctx, Distribution d, String taskId);
}
