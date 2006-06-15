package ru.novosoft.smsc.admin.console.commands.emailsme;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.AdminException;

public class EmailSmeDelCommand extends EmailSmeGenCommand{
    public void process(CommandContext ctx) {
        String out = "emailsme '" + address + "'";
        try {
            EmailSmeContext context = EmailSmeContext.getInstance(ctx.getOwner().getContext());
            context.delete(ton, npi, address);
            ctx.setMessage(out + " deleted.");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (AdminException e) {
            ctx.setMessage("Couldn't delete " + out + ". Cause: " + e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "EMAILSME_DEL";
    }
}
