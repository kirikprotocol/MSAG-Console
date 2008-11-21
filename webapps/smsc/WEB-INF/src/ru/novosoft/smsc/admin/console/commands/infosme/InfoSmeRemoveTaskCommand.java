package ru.novosoft.smsc.admin.console.commands.infosme;

import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.console.CommandContext;

/**
 * author: alkhal
 * Date: 21.11.2008
 */
public class InfoSmeRemoveTaskCommand extends CommandClass {

  private String taskId;

  public void process(CommandContext ctx) {
    try{
      final InfoSmeCommands cmd = (InfoSmeCommands)Class.forName("ru.novosoft.smsc.infosme.backend.commands.InfoSmeCommandsImpl").newInstance();
      cmd.removeTask(ctx, taskId);
    } catch (Throwable e) {
      e.printStackTrace();
      ctx.setMessage(e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "INFOSME_DELETE_MESSAGE";
  }

  public String getTaskId() {
    return taskId;
  }

  public void setTaskId(String taskId) {
    this.taskId = taskId;
  }
}
