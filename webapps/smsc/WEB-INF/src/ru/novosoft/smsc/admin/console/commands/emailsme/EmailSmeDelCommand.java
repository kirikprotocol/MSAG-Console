package ru.novosoft.smsc.admin.console.commands.emailsme;



import ru.novosoft.smsc.admin.console.CommandContext;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;


public class EmailSmeDelCommand extends EmailSmeGenCommand{

  public void process(CommandContext ctx) {

    String out = "emailsme '" + address + "'";

    try {

      process(ctx.getOwner().getContext());

      ctx.setMessage(out + " deleted.");

      ctx.setStatus(CommandContext.CMD_OK);

    } catch (AdminException e) {

      ctx.setMessage("Couldn't delete " + out + ". Cause: " + e.getMessage());

      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);

    }

  }

  public void process(SMSCAppContext ctx) throws AdminException {
    EmailSmeContext context = EmailSmeContext.getInstance(ctx);
    context.delete(ton, npi, address);
  }



  public String getId() {

    return "EMAILSME_DEL";

  }

}

