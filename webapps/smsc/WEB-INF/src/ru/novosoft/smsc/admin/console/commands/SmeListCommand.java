package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.Route;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;

import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.04.2005
 * Time: 17:42:28
 * To change this template use File | Settings | File Templates.
 */
public class SmeListCommand implements Command
{
    public void process(CommandContext ctx)
    {
        try
        {
            SmeManager manager = ctx.getSmeManager();
            Iterator i = manager.getSmeNames().iterator();
            if (!i.hasNext()) {
                ctx.setMessage("No SMEs defined");
                ctx.setStatus(CommandContext.CMD_OK);
            } else {
                while (i.hasNext()) {
                    String smeId = (String)i.next();
                    if (smeId != null && smeId.length() > 0) ctx.addResult(smeId);
                }
                ctx.setMessage("SMEs list");
                ctx.setStatus(CommandContext.CMD_LIST);
            }
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't list SMEs. Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }
    }

    public String getId() {
        return "SME_LIST";
    }
}
