package ru.novosoft.smsc.admin.console.commands.emailsme;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;

/**
 * User: artem
 * Date: 12.09.2006
 */
public class EmailSmeLookupByUserIdCommand extends CommandClass {

  private String userId = "";

  public void process(CommandContext ctx) {

    String out = "emailsme '" + userId + "'";

    try {

      final LookupResult result = process(ctx.getOwner().getContext());

      ctx.setMessage(out + '\n' + " lookup result: \n" + ((result == null) ? "nothing" : result.toString()));

      ctx.setStatus(CommandContext.CMD_OK);

    } catch (AdminException e) {

      ctx.setMessage("Couldn't find " + out + ". Cause: " + e.getMessage());

      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);

    }

  }

  public LookupResult process(SMSCAppContext ctx) throws AdminException {
    EmailSmeContext context = EmailSmeContext.getInstance(ctx);
    return context.lookupByUserId(userId);
  }

  public String getId() {
    return "EMAILSME_LOOKUP_BY_USER_ID";
  }

  public void setUserId(String userId) {
    this.userId = userId;
  }
}
