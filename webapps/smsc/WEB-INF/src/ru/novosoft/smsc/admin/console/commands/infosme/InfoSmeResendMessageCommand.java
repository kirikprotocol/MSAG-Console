package ru.novosoft.smsc.admin.console.commands.infosme;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;

/**
 * author: alkhal
 * Date: 11.11.2008
 */
public class InfoSmeResendMessageCommand extends CommandClass {

  private String msisdn;

  private String taskId;

  public void process(CommandContext ctx) {
    try{
      final InfoSmeCommands cmd = (InfoSmeCommands)Class.forName("ru.novosoft.smsc.infosme.backend.commands.InfoSmeCommandsImpl").newInstance();
      cmd.resendMessage(ctx, msisdn, taskId);
    } catch (Throwable e) {
      e.printStackTrace();
      ctx.setMessage(e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
    }
  }

  public String getId() {
    return "INFOSME_RESEND_MESSAGE";
  }

  public String getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(String msisdn) {
    this.msisdn = msisdn;
  }

  public String getTaskId() {
    return taskId;
  }

  public void setTaskId(String taskId) {
    this.taskId = taskId;
  }
}
